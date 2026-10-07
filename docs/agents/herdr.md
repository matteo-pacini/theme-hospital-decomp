# Multi-agent sessions under herdr

For a session where a lead agent briefs worker agents in herdr panes
(`HERDR_ENV=1`). Everything in AGENTS.md still applies.

## Workers

- Do every item of your brief. If one looks out of reach, say why with
  evidence (file:line in `src/`, or the level data) and ask the lead. Never
  narrow the task quietly or report done with items left out.
- "It can't happen" needs proof: the original's code and the data, ideally a
  run of the instrumented original showing it. Otherwise write the scenario.
- A replay that agrees proves nothing if nothing happened in it. Report the
  event counts that show each branch ran; check that each command in your
  scripts changes something in both traces.
- Shared files take small edits on top of what is there now. Never write a
  whole file from an earlier copy or from memory, never revert or delete code
  or files you didn't write. Re-read before each edit.
- No git commands. No broad `pkill`/`kill`: stop only processes you started.
- Your own build folder, `REPLAY_DIR` and temp folder, as the brief names.
- When done, write the report file the brief names and reply only
  `DONE <path>`.

## Lead

- Brief with a common rules file and one file per worker: task, gates, report
  path. Name each herdr agent after its pane label; if herdr drops a name,
  address the pane id.
- Verify every handoff independently (a subagent comparing the work with the
  original, running its scripts and tests) before accepting it, and again
  after a substantial fix round. Accept nothing on the worker's word,
  least of all a scope cut or an "impossible" claim.
- Finish a batch with one combined check of the whole tree: every replay
  script on both sides, every drive test, all gates.
- Wait for workers going idle or blocked (`herdr agent get`), not for their
  report files: they update reports while working.
- Give unrelated tasks a fresh session (`/new` in opencode). When a worker
  passes about 70% of its context mid-task, have it write its notes to a file
  and continue in a fresh session.
- Answer workers' permission dialogs only with the user's go-ahead. A watcher
  approves in-scope requests (the repository, `/tmp`, reads, build and replay
  tools) and rejects writing git commands, deletions outside the repository,
  killing others' processes and anything touching credentials; it logs every
  decision.
- After each round, check `git diff` on shared files (`main.cpp`, `game.cpp`,
  `staff.cpp`, `CMakeLists.txt`, ...) for deletions nobody explained: a stale
  whole-file write erases others' work silently.

## Observed tendencies

Seen with DeepSeek V4.1 Flash workers; the rules above guard against them
whatever the model:

- Ports faithfully and quickly when it does the work; cheap per task.
- Reports "done" with the hardest items skipped or described as out of scope.
- States wrong "not possible" conclusions with confidence.
- Trusts green gates as proof: scripts that agree while exercising nothing.
- Under concurrency, once overwrote a shared file with a stale copy.
