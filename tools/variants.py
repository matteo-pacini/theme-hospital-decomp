#!/usr/bin/env python3
"""Search source variants of one function for a better match.

Starting from the function as written, each round tries every single
change of the kinds selected on each of the best few variants so far (a
beam, since a fix often needs two changes neither of which helps alone),
scores them (objdiff's match percentage for that function, as in the
report) and keeps the best few, until one matches or nothing improves. Most remaining differences are register or
stack-slot choice, which these changes steer without altering what the code
does:

  swaps    commutative operands (`a + b` -> `b + a`, `a < b` -> `b > a`)
  decls    the order of two declarations in the same group
  opassign `x op= e` <-> `x = x op (e)`
  types    a local's integer type (any width or signedness: review these,
           since a different type can change behaviour)
  blocks   a declaration moved into an inner block that holds every use,
           or out of a nested block to the function's top-level
           declarations (the compiler sorts a block's locals separately)
  zero     `!x` <-> `x == 0`, `x` <-> `x != 0` in conditions
  adjacent two neighbouring simple statements swapped (review these: the
           order can matter)
  ifelse   `if (c) {A} else {B}` -> `if (!c) {B} else {A}`
  reuse    one local of the same type standing in for another, in the
           whole body (dropping the other's declaration) or in one block
           (review these: a merge can change behaviour)

Nothing under src/ is written: the best variant is printed as a diff, and
--out saves the whole file.

Usage: tools/variants.py SOURCE 0xADDR [--kinds swaps,decls,opassign,blocks,zero]
                         [--rounds N] [--beam N] [--out FILE] [--from FILE]

--from starts from another copy of SOURCE (a draft kept outside the tree);
SOURCE still decides the unit and the compiler flags.
"""

import argparse
import bisect
import concurrent.futures
import difflib
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.dirname(__file__))
import cc  # noqa: E402
from lefile import LEFile  # noqa: E402
from project import EXE, ROOT, Project  # noqa: E402
from split import write_unit  # noqa: E402

WORK = os.path.join(ROOT, "build", "variants")

# ---------------------------------------------------------------- scoring


def target_object(unit):
    """The unit's target object, cached for the current configuration."""
    key = hashlib.md5(b"".join(
        open(os.path.join(ROOT, "config", c), "rb").read()
        for c in ("functions.txt", "symbols.txt", "relocs.txt", "sections.txt")
    )).hexdigest()[:12]
    path = os.path.join(WORK, "target", key, unit.name.replace("/", "_"))
    if not os.path.exists(path + ".o"):
        os.makedirs(os.path.dirname(path), exist_ok=True)
        write_unit(LEFile(EXE), Project(), unit, path + ".s")
        subprocess.run(["as", "--32", "-o", path + ".o", path + ".s"], check=True, capture_output=True)
    return path + ".o"


INCLUDE = re.compile(r'^(\s*#\s*include\s*)"([^"]+)"', re.M)


def rebase_includes(text, source):
    """Quoted includes found next to the source rewritten relative to the
    repository root, since the variant is compiled from another directory."""
    here = os.path.dirname(source)

    def rebase(m):
        path = os.path.normpath(os.path.join(here, m.group(2)))
        if not os.path.exists(os.path.join(ROOT, path)):
            return m.group(0)
        return f'{m.group(1)}"{cc.dos_path(path)}"'

    return INCLUDE.sub(rebase, text)


class Scorer:
    def __init__(self, source, fn, unit):
        self.source, self.fn = source, fn
        self.target = target_object(unit)
        self.command = cc.command(*cc.compiler_for(source)) + ["-i=."]

    def __call__(self, job):
        n, text = job
        d = os.path.relpath(os.path.join(WORK, f"w{os.getpid()}_{n}"), ROOT)
        os.makedirs(d, exist_ok=True)
        try:
            src = os.path.join(d, "v" + os.path.splitext(self.source)[1])
            open(os.path.join(ROOT, src), "w", encoding="latin-1").write(rebase_includes(text, self.source))
            omf, elf = os.path.join(d, "v.obj"), os.path.join(d, "v.o")
            r = subprocess.run(
                [*self.command, f"-fo={cc.dos_path(omf)}", cc.dos_path(src)],
                capture_output=True, text=True, cwd=ROOT,
            )
            if r.returncode:
                return -1.0
            cc.omf2elf.main(os.path.join(ROOT, omf), os.path.join(ROOT, elf))
            out = subprocess.run(
                ["objdiff-cli", "diff", "-1", self.target, "-2", os.path.join(ROOT, elf), "-o", "-",
                 "--format", "json", self.fn.name],
                capture_output=True, text=True,
            ).stdout
            sym = next(s for s in json.loads(out)["left"]["symbols"] if s["name"] == self.fn.name)
            return float(sym.get("match_percent", 0.0))
        except (StopIteration, ValueError, KeyError, OSError):
            return -1.0
        finally:
            shutil.rmtree(os.path.join(ROOT, d), ignore_errors=True)


# ---------------------------------------------------------------- changes

TOK = re.compile(
    r"\s+|//[^\n]*|/\*.*?\*/|\"(?:\\.|[^\"\\])*\"|'(?:\\.|[^'\\])*'|0[xX][0-9a-fA-F]+[uUlL]*|\d+[uUlL]*"
    r"|[A-Za-z_]\w*|->|\+\+|--|<<=|>>=|<<|>>|<=|>=|==|!=|&&|\|\||[-+*/%&|^]=|.",
    re.S,
)
BIN = {"*": 13, "/": 13, "%": 13, "+": 12, "-": 12, "<<": 11, ">>": 11, "<": 10, ">": 10, "<=": 10,
       ">=": 10, "==": 9, "!=": 9, "&": 8, "^": 7, "|": 6, "&&": 5, "||": 4}
ASSIGN = {"=", "+=", "-=", "*=", "/=", "|=", "&=", "^=", "<<=", ">>=", "%="}
SWAP = {"+": "+", "*": "*", "&": "&", "|": "|", "^": "^", "==": "==", "!=": "!=",
        "<": ">", ">": "<", "<=": ">=", ">=": "<="}
KEYWORDS = {"return", "case", "if", "while", "for", "switch", "else", "do", "goto", "break", "continue",
            "default", "sizeof"}
TYPES = set("u8 s8 u16 s16 u32 s32 int char long short unsigned signed void const volatile".split())


def tokens(s):
    return [(m.start(), m.end(), m.group(0)) for m in TOK.finditer(s)
            if not (m.group(0).isspace() or m.group(0).startswith(("//", "/*")))]


class Parser:
    """Just enough of C's expression grammar to find binary operators."""

    def __init__(self, toks):
        self.T, self.nodes = toks, []

    def t(self, k):
        return self.T[k][2] if k < len(self.T) else ""

    def close(self, k):
        o = self.t(k)
        c = {"(": ")", "[": "]", "{": "}"}[o]
        depth = 0
        for q in range(k, len(self.T)):
            if self.T[q][2] == o:
                depth += 1
            elif self.T[q][2] == c:
                depth -= 1
                if depth == 0:
                    return q
        raise ValueError("unbalanced")

    def is_cast(self, k):
        inner = [x[2] for x in self.T[k + 1 : self.close(k)]]
        return bool(inner) and (inner[0] in TYPES or re.match(r"[A-Z]", inner[0])) and all(
            x in TYPES or x == "*" or (re.match(r"[A-Z]\w*$", x) and x != x.upper()) for x in inner)

    def unary(self, k):
        t = self.t(k)
        if t in ("-", "!", "~", "*", "&", "+", "++", "--"):
            return self.unary(k + 1)
        if t == "(" and self.is_cast(k):
            return self.unary(self.close(k) + 1)
        if t == "sizeof":
            return self.close(k + 1) + 1 if self.t(k + 1) == "(" else self.unary(k + 1)
        return self.postfix(k)

    def postfix(self, k):
        t = self.t(k)
        if t == "(":
            q = self.close(k)
            self.expr(k + 1)
            k = q + 1
        elif re.match(r"[A-Za-z_]\w*$|\d|0[xX]|[\"']", t):
            k += 1
        else:
            raise ValueError(t)
        while True:
            t = self.t(k)
            if t == "[":
                q = self.close(k)
                self.expr(k + 1)
                k = q + 1
            elif t == "(":
                q, a = self.close(k), k + 1
                while a < q:
                    a = self.assign(a)
                    if self.t(a) != ",":
                        break
                    a += 1
                k = q + 1
            elif t in (".", "->"):
                k += 2
            elif t in ("++", "--"):
                k += 1
            else:
                return k

    def binary(self, k, min_prec):
        end = self.unary(k)
        lhs = (k, end, None)
        while True:
            op = self.t(end)
            if op not in BIN or BIN[op] < min_prec:
                return end, lhs
            end2, rhs = self.binary(end + 1, BIN[op] + 1)
            node = (lhs[0], end2, op, end, lhs, rhs)
            self.nodes.append(node)
            lhs, end = (lhs[0], end2, node), end2

    def conditional(self, k):
        end, _ = self.binary(k, 4)
        if self.t(end) == "?":
            end = self.assign(end + 1)
            if self.t(end) != ":":
                raise ValueError("?:")
            end = self.conditional(end + 1)
        return end

    def assign(self, k):
        end = self.conditional(k)
        return self.assign(end + 1) if self.t(end) in ASSIGN else end

    def expr(self, k):
        end = self.assign(k)
        while self.t(end) == ",":
            end = self.assign(end + 1)
        return end


def swaps(body):
    T = tokens(body)
    p = Parser(T)
    starts = [0] + [k + 1 for k, x in enumerate(T) if x[2] in (";", "{", "}", "(", ":", "?", ",", "=", "return", "case", "else")]
    for k in starts:
        if k < len(T) and T[k][2] not in KEYWORDS - {"sizeof"} and T[k][2] not in TYPES:
            try:
                p.expr(k)
            except (ValueError, KeyError):
                pass
    seen = {(n[0], n[1], n[3]): n for n in p.nodes}
    text = lambda a, e: body[T[a][0] : T[e - 1][1]]
    for a, e, op, at, lhs, rhs in seen.values():
        if op not in SWAP:
            continue
        left, right = text(lhs[0], lhs[1]), text(at + 1, e)
        if lhs[2] is not None and BIN[lhs[2][2]] <= BIN[op]:
            left = f"({left})"
        if rhs is not None and rhs[2] is not None and BIN[rhs[2][2]] <= BIN[op]:
            right = f"({right})"
        yield f"swap {text(a, e)}", body[: T[a][0]] + f"{right} {SWAP[op]} {left}" + body[T[e - 1][1] :]


DECL = re.compile(r"^(\s*)((?:const |volatile |unsigned |signed )*[A-Za-z_]\w*[\s*]+)([A-Za-z_]\w*)(\[[^\]]*\])?((?: = [^;]+)?);\s*$")


def decl_groups(lines):
    group = []
    for i, line in enumerate(lines):
        m = DECL.match(line)
        if m and m.group(2).split()[0] not in KEYWORDS:
            group.append(i)
            continue
        if len(group) > 1:
            yield group
        group = []
    if len(group) > 1:
        yield group


def decls(body):
    lines = body.split("\n")
    for group in decl_groups(lines):
        for x in range(len(group)):
            for y in range(x + 1, len(group)):
                i, j = group[x], group[y]
                new = list(lines)
                new[i], new[j] = new[j], new[i]
                yield f"decls {lines[i].strip()} <-> {lines[j].strip()}", "\n".join(new)


OPASSIGN = re.compile(r"^(\s*)([\w.\[\]>\-]+) ([-+*/%&|^]|<<|>>)= (.+);\s*$")
EXPANDED = re.compile(r"^(\s*)([\w.\[\]>\-]+) = \2 ([-+*/%&|^]|<<|>>) \((.+)\);\s*$")


def opassign(body):
    lines = body.split("\n")
    for i, line in enumerate(lines):
        m = OPASSIGN.match(line) or EXPANDED.match(line)
        if not m:
            continue
        ind, lhs, op, rhs = m.groups()
        repl = f"{ind}{lhs} = {lhs} {op} ({rhs});" if OPASSIGN.match(line) else f"{ind}{lhs} {op}= {rhs};"
        yield f"opassign {line.strip()}", "\n".join(lines[:i] + [repl] + lines[i + 1 :])


INTEGERS = ("s8", "u8", "s16", "u16", "s32", "u32")


def types(body):
    lines = body.split("\n")
    for i, line in enumerate(lines):
        m = DECL.match(line)
        current = m.group(2).strip() if m else None
        if current not in INTEGERS + ("int",):
            continue
        for w in INTEGERS:
            if w == current or (current == "int" and w == "s32"):
                continue
            repl = f"{m.group(1)}{w} {m.group(3)}{m.group(4) or ''}{m.group(5)};"
            yield f"types {line.strip()} -> {w}", "\n".join(lines[:i] + [repl] + lines[i + 1 :])


def brace_pairs(body):
    """(open, close) offsets of every brace pair in body."""
    stack, pairs = [], []
    for a, _, t in tokens(body):
        if t == "{":
            stack.append(a)
        elif t == "}" and stack:
            pairs.append((stack.pop(), a))
    return pairs


def blocks(body):
    lines = body.split("\n")
    starts = [0]
    for line in lines:
        starts.append(starts[-1] + len(line) + 1)
    pairs = brace_pairs(body)
    for i, line in enumerate(lines):
        m = DECL.match(line)
        if not m or m.group(2).split()[0] in KEYWORDS:
            continue
        name, decl = m.group(3), line.strip()
        lo, hi = starts[i], starts[i + 1]
        uses = [u.start() for u in re.finditer(rf"(?<![\w.>]){re.escape(name)}\b", body) if not lo <= u.start() < hi]
        if not uses:
            continue
        enclosing = max((a for a, c in pairs if a < lo and c >= hi), default=-1)
        rest = lines[:i] + lines[i + 1 :]
        for a, c in pairs:
            if not (a > enclosing and a < min(uses) and c > max(uses)):
                continue
            # Insert after the line holding the `{` (numbered in `rest`).
            j = bisect.bisect_right(starts, a) - 1
            k = j + 1 if j < i else j
            if k >= len(rest):
                continue
            indent = rest[k][: len(rest[k]) - len(rest[k].lstrip())]
            if not indent:
                continue
            yield f"blocks {name} -> line {k}", "\n".join(rest[:k] + [indent + decl] + rest[k:])
        if enclosing >= 0:
            top = 0
            while top < len(rest) and DECL.match(rest[top]) and len(DECL.match(rest[top]).group(1)) == 4:
                top += 1
            for k in range(top + 1):
                yield f"blocks {name} -> top {k}", "\n".join(rest[:k] + ["    " + decl] + rest[k:])


LVALUE = r"[A-Za-z_][\w.\[\]>\-]*"
ZERO_TESTS = [
    (re.compile(rf"\b(if|while) \(!({LVALUE})\)"), lambda m: f"{m[1]} ({m[2]} == 0)"),
    (re.compile(rf"\b(if|while) \(({LVALUE}) == 0\)"), lambda m: f"{m[1]} (!{m[2]})"),
    (re.compile(rf"\b(if|while) \(({LVALUE})\)"), lambda m: f"{m[1]} ({m[2]} != 0)"),
    (re.compile(rf"\b(if|while) \(({LVALUE}) != 0\)"), lambda m: f"{m[1]} ({m[2]})"),
    (re.compile(rf"([(&|] *)!({LVALUE})(?=\s*[)&|])"), lambda m: f"{m[1]}{m[2]} == 0"),
]


def zero(body):
    for rx, repl in ZERO_TESTS:
        for m in rx.finditer(body):
            yield f"zero {m[0]}", body[: m.start()] + repl(m) + body[m.end() :]


SIMPLE = re.compile(r"^(\s+)(?!(?:return|break|goto|continue|case|default|if|else|for|while|do|switch)\b)[^{}]*;\s*$")


def adjacent(body):
    lines = body.split("\n")
    for i in range(len(lines) - 1):
        a, b = SIMPLE.match(lines[i]), SIMPLE.match(lines[i + 1])
        if a and b and a.group(1) == b.group(1) and not DECL.match(lines[i]) and not DECL.match(lines[i + 1]):
            new = list(lines)
            new[i], new[i + 1] = new[i + 1], new[i]
            yield f"adjacent {lines[i].strip()} <-> {lines[i + 1].strip()}", "\n".join(new)


NEGATED = {"==": "!=", "!=": "==", "<": ">=", ">=": "<", ">": "<=", "<=": ">"}


def negate(cond):
    cond = cond.strip()
    if m := re.fullmatch(rf"!({LVALUE})", cond):
        return m[1]
    if re.fullmatch(LVALUE, cond):
        return "!" + cond
    m = re.fullmatch(r"([^()&|?]+?) (==|!=|<=|>=|<|>) ([^()&|?]+)", cond)
    return f"{m[1]} {NEGATED[m[2]]} {m[3]}" if m else f"!({cond})"


def ifelse(body):
    lines = body.split("\n")
    for i, line in enumerate(lines):
        m = re.match(r"^(\s*)(\} else )?if \((.*)\) \{$", line)
        if not m:
            continue
        indent, els, end = m[1], None, None
        for k in range(i + 1, len(lines)):
            if lines[k].startswith(indent + "}"):
                if lines[k].strip() == "} else {":
                    els = k
                    continue
                end = k if lines[k].strip() == "}" else None
                break
        if els is None or end is None:
            continue
        head = f"{indent}{m[2] or ''}if ({negate(m[3])}) {{"
        new = lines[:i] + [head] + lines[els + 1 : end] + [indent + "} else {"] + lines[i + 1 : els] + lines[end:]
        yield f"ifelse {line.strip()}", "\n".join(new)


def reuse(body):
    lines = body.split("\n")
    scalars = []
    for i, line in enumerate(lines):
        m = DECL.match(line)
        if m and m.group(2).split()[0] not in KEYWORDS and not m.group(4):
            scalars.append((i, m.group(2).strip(), m.group(3)))
    pairs = brace_pairs(body)
    for _, type_a, a in scalars:
        for j, type_b, b in scalars:
            if a == b or type_a != type_b:
                continue
            name = re.compile(rf"(?<![\w.>]){re.escape(b)}\b")
            yield f"reuse {b} -> {a}", name.sub(a, "\n".join(lines[:j] + lines[j + 1 :]))
            for lo, hi in pairs:
                block = body[lo : hi + 1]
                if name.search(block):
                    yield f"reuse {b} -> {a} in block at {lo}", body[:lo] + name.sub(a, block) + body[hi + 1 :]


KINDS = {"swaps": swaps, "decls": decls, "opassign": opassign, "types": types,
         "blocks": blocks, "zero": zero, "adjacent": adjacent, "ifelse": ifelse, "reuse": reuse}

# ---------------------------------------------------------------- search


def function_body(text, addr):
    """(prefix, body, suffix) around the body of the function marked addr."""
    m = re.search(rf"//\s*FUNCTION:\s*0x0*{addr:x}\b", text, re.I)
    if not m:
        sys.exit(f"no FUNCTION marker for {addr:#x}")
    i = text.index("\n{\n", m.end()) + 3
    j = text.index("\n}\n", i) + 1
    return text[:i], text[i:j], text[j:]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("source")
    ap.add_argument("addr", type=lambda s: int(s, 16))
    ap.add_argument("--kinds", default="swaps,decls,opassign")
    ap.add_argument("--rounds", type=int, default=6)
    ap.add_argument("--beam", type=int, default=4)
    ap.add_argument("--out")
    ap.add_argument("--from", dest="start")
    args = ap.parse_args()

    source = os.path.relpath(os.path.abspath(args.source), ROOT)
    proj = Project()
    unit = next(u for u in proj.units if u.source == source)
    fn = next(f for f in unit.functions if f.addr == args.addr)
    original = open(args.start or os.path.join(ROOT, source), encoding="latin-1").read()
    prefix, body, suffix = function_body(original, args.addr)
    score = Scorer(source, fn, unit)
    kinds = [KINDS[k] for k in args.kinds.split(",")]

    base = score((0, original))
    print(f"{fn.name}: {base:.2f}%", flush=True)
    beam = [(base, body, [])]  # (score, body, changes applied)
    seen = {body}
    with concurrent.futures.ThreadPoolExecutor(os.cpu_count()) as pool:
        for _ in range(args.rounds):
            if beam[0][0] >= 100.0:
                break
            cands = []
            for _, b, steps in beam:
                for kind in kinds:
                    for label, nb in kind(b):
                        if nb not in seen:
                            seen.add(nb)
                            cands.append((nb, steps + [label]))
            if not cands:
                break
            results = pool.map(score, ((n + 1, prefix + nb + suffix) for n, (nb, _) in enumerate(cands)))
            scored = sorted(((r, nb, steps) for r, (nb, steps) in zip(results, cands)), key=lambda x: -x[0])
            merged = sorted(beam + scored[: args.beam], key=lambda x: -x[0])[: args.beam]
            if merged[0][0] <= beam[0][0] and [x[0] for x in merged] == [x[0] for x in beam]:
                break
            beam = merged
            print(f"  {beam[0][0]:.2f}%  {'; '.join(beam[0][2])}  ({len(cands)} tried)", flush=True)
    best, body, _ = beam[0]

    result = prefix + body + suffix
    if result != original:
        sys.stdout.writelines(difflib.unified_diff(
            original.splitlines(True), result.splitlines(True), source, source + " (best)"))
        if args.out:
            open(args.out, "w", encoding="latin-1").write(result)
    print(f"best {best:.2f}%")


if __name__ == "__main__":
    main()
