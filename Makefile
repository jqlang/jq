jq: *.hs Parser.hs Lexer.hs
	ghc *.hs -o jq

Parser.hs: Parser.y
	happy -i Parser.y

Lexer.hs: Lexer.x
	alex Lexer.x