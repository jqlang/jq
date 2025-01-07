function Pandoc(el)
  el.blocks:walk{Header = function(el)
    print("  - level: " .. el.level)
    print("    title: '" .. pandoc.utils.stringify(el) .. "'")
    print("    id: " .. el.identifier)
  end}
  os.exit(0)
end
