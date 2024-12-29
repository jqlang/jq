function Pandoc(el)
  el.blocks:walk{Div = function(el)
    if el.classes:includes'Examples' then
      el:walk{CodeBlock = function(block) print(block.text .. "\0") end}
    end
  end}
  os.exit(0)
end
