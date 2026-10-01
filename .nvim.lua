-- Project settings for Neovim (read when 'exrc' is on; Neovim asks to trust
-- the file the first time).
--
-- Plain clang-format rewrites Watcom pragmas (`parm [eax]` -> `parm[eax]`),
-- so format-on-save is off for this project's sources; format them with
-- `tools/format.py FILE`, which keeps every #pragma as written.
-- clangd and clang-tidy find their flags in compile_commands.json, which
-- `python3 configure.py` writes.

local root = vim.fn.fnamemodify(debug.getinfo(1, "S").source:sub(2), ":p:h")

vim.api.nvim_create_autocmd({ "BufReadPost", "BufNewFile" }, {
  pattern = { root .. "/*.c", root .. "/*.cpp", root .. "/*.h" },
  callback = function(args)
    vim.b[args.buf].disableFormatSave = true
  end,
})
