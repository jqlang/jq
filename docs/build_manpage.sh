#!/bin/sh
pandoc man/prologue.md content/manual/manual.md man/epilogue.md -s --to=man --lua-filter filters/filter.lua
