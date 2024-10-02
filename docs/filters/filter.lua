--[[
function dump(o)
   if type(o) == 'table' then
      local s = '{ '
      for k,v in pairs(o) do
         if type(k) ~= 'number' then k = '"'..k..'"' end
         s = s .. '['..k..'] = ' .. dump(v) .. ','
      end
      return s .. '} '
   else
      return tostring(o)
   end
end
]]--

-- uppercase top-level headings for man pages
function Header(el)
  if FORMAT == "man" and el.level == 1 then
    return el:walk{Str = function(el) return pandoc.Str(string.upper(el.text)) end}
  end
end

-- inline code is assumed to be in jq
function Code(code)
  code.classes[1] = "jq"
  return code
end

-- code blocks are assumed to be in jq if no other language is given
function CodeBlock(block)
  if next(block.classes) == nil then
    block.classes[1] = "jq"
    return block
  end
end

function Div(el)
  if el.classes:includes'Examples' then
    local el = el:walk{CodeBlock = function(block) return example(block.text) end}

    if FORMAT == "html" then
      local summary = pandoc.Plain{
        pandoc.RawInline("html",  "<summary>"), pandoc.Str 'Examples',
        pandoc.RawInline("html", "</summary>")
      }
      return {
        pandoc.RawBlock("html",  '<details class="pb-3">'), summary, el,
        pandoc.RawBlock("html", '</details>')
      }
    end
    return el.content
  end
  if #el.classes == 1 then
    local class = el.classes[1]
    el.classes = {"alert", "alert-primary"}
    table.insert(el.content, 1, pandoc.Span(class, {class = "block-start"}))
  end
  return el
end

function example(test)
  local _, _, filter, input, output = test:find("([^\n]+)\n([^\n]+)\n(.*)")

  if FORMAT == "man" then
    input = "echo '" .. input .. "'"
    filter = "jq '" .. filter .. "'"
    return pandoc.CodeBlock("$ " .. input .. " \\\n  | " .. filter .. "\n" .. output)
  end

  local url = "https://jqplay.org/jq?q=" .. encodeUrl(filter) .. "&j=" .. encodeUrl(input)
  simple_table = pandoc.SimpleTable(
    "", -- caption
    {pandoc.AlignRight, pandoc.AlignLeft},
    {0, 0}, -- let pandoc determine col widths,
    {}, -- headers
    {
      {pandoc.Strong("Filter"), pandoc.Code(filter, {class = "jq"  })},
      {pandoc.Strong( "Input"), pandoc.Code( input, {class = "json"})},
      {pandoc.Strong("Output"), pandoc.Code(output, {class = "json"})},
      {pandoc.Link("Run", url), {}}
    }
  )
  local table = pandoc.utils.from_simple_table(simple_table)
  if FORMAT == "html" then
    table.classes = {"table", "table-borderless", "table-sm", "w-auto"}
  end
  return table
end

function encodeUrl(str)
  str = string.gsub(str, "\n", "\r\n")
  str = string.gsub(str, "([^%w%.%- ])", function(c) return string.format("%%%02X", string.byte(c)) end)
  str = string.gsub(str, " ", "+")
  return str
end
