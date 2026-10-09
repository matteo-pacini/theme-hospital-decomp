# Multi-agent sessions under herdr

For a session where a lead agent briefs worker agents in herdr panes
(`HERDR_ENV=1`). Everything in AGENTS.md still applies, including running
`herdr --skill` before any herdr operation.

## Workspaces and worktrees

- The default setup comes first: the main workspace, with `th-lead` in the
  main repository on whatever branch it has checked out, and its workers
  `th-agent-01` to `th-agent-07`. Most work runs there, split among the
  workers.
- A big task unrelated to the main workspace's work can get its own herdr
  workspace on its own git worktree, on demand, to be merged back when done.
  When it is unclear which fits, ask the user whether to open a worktree
  workspace or split the task among the available workers.
- When the lead is asked to set up the tabs in a new worktree workspace, it
  first asks the user what work the workspace is for and proposes a short
  tag from the answer (`rivals` for porting the computer hospitals). Its
  agents are then `th-lead-<tag>` and `th-agent-NN-<tag>`: herdr agent names
  are unique across every workspace, so the tag keeps them apart. Create
  the workspace from the main repository with:

      herdr worktree create --branch <tag> --base <branch> \
        --path ../theme-hospital-decomp-wt/<tag> --label <tag>

- Every worktree, a lead's or a worker's, links `orig`, `build/base`,
  `build/slots` and `build/replay-cache` to the main tree's, never copies:
  the slots cap DOSBox and driven runs for the whole machine, and the caches
  are keyed by content and written atomically, so every worktree adds to and
  reads one cache.
- Leads may message each other through herdr to coordinate (a shared file
  about to change, an `instrument.py` change that invalidates the trace
  cache once merged, the order of merges). Workers talk only to the lead of
  their own workspace, and only when it asks; a reply always goes to that
  lead.
- A worktree workspace's lead merges its branch back into the branch it
  started from when the user asks, coordinating with `th-lead`, then removes
  its worktrees and closes the workspace.

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
- Use your own build folder, `REPLAY_DIR` and temp folder, as the brief names
  them.
- Don't use herdr to message the lead or other agents unless the lead asks
  you to: the lead reads your pane. Questions and answers go on your own
  screen, or in a file when the lead asks for one.
- When done, clean up what you made (AGENTS.md, "Sharing the machine"),
  then write the report file the brief names and end with `DONE <path>`.

## Lead

- Set up (or recreate) this layout in the lead's workspace. Tabs `agents`
  and `agents-2` are 2x2 grids (split right, then each half down):

  | Tab        | Top left    | Bottom left | Top right   | Bottom right |
  |------------|-------------|-------------|-------------|--------------|
  | `agents`   | th-lead     | th-agent-02 | th-agent-01 | th-agent-03  |
  | `agents-2` | th-agent-04 | th-agent-06 | th-agent-05 | th-agent-07  |

  Then two tabs for the user, one pane each, in the repository root:
  `terminal` (a plain shell) and `nvim` (running `nvim .`). Label each agent
  pane after the agent in it.
- Start each worker in a pane at a shell prompt, named after
  its pane label, `th-agent-NN` (`th-agent-NN-<tag>` in a worktree
  workspace); if herdr drops a name, address the pane id:

      herdr agent start <name> --kind claude --pane <id> -- \
        --model claude-sonnet-5-5 --effort medium

  Send `/caveman` before the brief, and `/clear` before giving a worker an
  unrelated task.
- Brief with a common rules file and one file per worker: task, gates, report
  path.
- Verify every handoff before accepting it (rerun its scripts and tests,
  compare the work with the original, look at its screenshots), and again
  after a substantial fix round. Accept no scope cut or "impossible" claim
  without the evidence the Workers rules ask for. The lead makes the final
  decision.
- Per task, decide whether each worker gets its own git worktree
  (`theme-hospital-decomp-wt/<name>`, branch `<branch>/<name>`, linked as in
  "Workspaces and worktrees") or works in the lead's tree with its own
  build folder and `REPLAY_DIR`: its own worktree when workers would edit
  the same files, the shared tree for small, separate tasks. Workers never
  commit; the lead merges verified worktrees into its branch (through an
  integration worktree for a large batch), runs the combined check there
  and commits when the user asks.
- Finish a batch with one combined check of the whole tree: every replay
  script on both sides, every drive test, all gates. Then remove the batch's
  traces, verification folders and stale `build/replay-*` folders, and check
  free space on `/` (`/tmp` lives there).
- Wait for workers going idle or blocked (`herdr agent get`), not for their
  report files: they update reports while working.
- When a worker passes about 70% of its context mid-task, have it write its
  notes to a file and continue after `/clear` from them.
- Answer workers' permission dialogs only with the user's go-ahead. With
  it, the lead may run a watcher (a loop reading the workers' screens) that
  approves in-scope requests (the repository, `/tmp`, reads, build and replay
  tools) and rejects writing git commands, deletions outside the repository,
  killing others' processes and anything touching credentials; it logs every
  decision.
- After each round, check `git diff` on shared files (`main.cpp`, `game.cpp`,
  `staff.cpp`, `CMakeLists.txt`, ...) for deletions nobody explained: a stale
  whole-file write erases others' work silently.
