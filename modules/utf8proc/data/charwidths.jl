# Following work by @jiahao, we compute character widths using a combination of
#   * character category
#   * UAX 11: East Asian Width
#   * a few exceptions as needed
# Adapted from http://nbviewer.ipython.org/gist/jiahao/07e8b08bf6d8671e9734
#
# We used to also use data from GNU Unifont, but that has proven unreliable
# and unlikely to match widths assumed by terminals.
#
# Requires Julia (obviously) and FontForge.

#############################################################################
CharWidths = Dict{Int,Int}()

#############################################################################
# Use ../libutf8proc for category codes, rather than the one in Julia,
# to minimize bootstrapping complexity when a new version of Unicode comes out.
catcode(c) = ccall((:utf8proc_category,"../libutf8proc"), Cint, (Int32,), c)

# utf8proc category constants (must match h)
const UTF8PROC_CATEGORY_CN = 0
const UTF8PROC_CATEGORY_LU = 1
const UTF8PROC_CATEGORY_LL = 2
const UTF8PROC_CATEGORY_LT = 3
const UTF8PROC_CATEGORY_LM = 4
const UTF8PROC_CATEGORY_LO = 5
const UTF8PROC_CATEGORY_MN = 6
const UTF8PROC_CATEGORY_MC = 7
const UTF8PROC_CATEGORY_ME = 8
const UTF8PROC_CATEGORY_ND = 9
const UTF8PROC_CATEGORY_NL = 10
const UTF8PROC_CATEGORY_NO = 11
const UTF8PROC_CATEGORY_PC = 12
const UTF8PROC_CATEGORY_PD = 13
const UTF8PROC_CATEGORY_PS = 14
const UTF8PROC_CATEGORY_PE = 15
const UTF8PROC_CATEGORY_PI = 16
const UTF8PROC_CATEGORY_PF = 17
const UTF8PROC_CATEGORY_PO = 18
const UTF8PROC_CATEGORY_SM = 19
const UTF8PROC_CATEGORY_SC = 20
const UTF8PROC_CATEGORY_SK = 21
const UTF8PROC_CATEGORY_SO = 22
const UTF8PROC_CATEGORY_ZS = 23
const UTF8PROC_CATEGORY_ZL = 24
const UTF8PROC_CATEGORY_ZP = 25
const UTF8PROC_CATEGORY_CC = 26
const UTF8PROC_CATEGORY_CF = 27
const UTF8PROC_CATEGORY_CS = 28
const UTF8PROC_CATEGORY_CO = 29

#############################################################################
# Use a default width of 1 for all character categories that are
# letter/symbol/number-like, as well as for unassigned/private-use chars.
# This can be overridden by UAX 11
# below, but provides a useful nonzero fallback for new codepoints when
# a new Unicode version has been released but Unifont hasn't been updated yet.

zerowidth = Set{Int}() # categories that may contain zero-width chars
push!(zerowidth, UTF8PROC_CATEGORY_MN)
push!(zerowidth, UTF8PROC_CATEGORY_MC)
push!(zerowidth, UTF8PROC_CATEGORY_ME)
# push!(zerowidth, UTF8PROC_CATEGORY_SK)  # see issue #167
push!(zerowidth, UTF8PROC_CATEGORY_ZL)
push!(zerowidth, UTF8PROC_CATEGORY_ZP)
push!(zerowidth, UTF8PROC_CATEGORY_CC)
push!(zerowidth, UTF8PROC_CATEGORY_CF)
push!(zerowidth, UTF8PROC_CATEGORY_CS)
for c in 0x0000:0x110000
    if catcode(c) ∉ zerowidth
        CharWidths[c] = 1
    end
end

#############################################################################
# Widths from UAX #11: East Asian Width
#   .. these take precedence for all codepoints
#      listed explicitly as wide/full/narrow/half-width

for line in readlines(open("EastAsianWidth.txt"))
    #Strip comments
    (isempty(line) || line[1] == '#') && continue
    precomment = split(line, '#')[1]
    #Parse code point range and width code
    tokens = split(precomment, ';')
    length(tokens) >= 2 || continue
    charrange = tokens[1]
    width = strip(tokens[2])
    #Parse code point range into Julia UnitRange
    rangetokens = split(charrange, "..")
    charstart = parse(UInt32, "0x"*rangetokens[1])
    charend = parse(UInt32, "0x"*rangetokens[length(rangetokens)>1 ? 2 : 1])

    #Assign widths
    for c in charstart:charend
        if width=="W" || width=="F" # wide or full
            CharWidths[c]=2
        elseif width=="Na"|| width=="H"
            CharWidths[c]=1
        end
    end
end

#############################################################################
# A few exceptions to the above cases, found by manual comparison
# to other wcwidth functions and similar checks.

for c in keys(CharWidths)
    cat = catcode(c)

    # make sure format control character (category Cf) have width 0
    # (some of these, like U+0601, can have a width in some cases
    #  but normally act like prepended combining marks.  U+fff9 etc
    #  are also odd, but have zero width in typical terminal contexts)
    if cat==UTF8PROC_CATEGORY_CF
        CharWidths[c]=0
    end

    # Unifont has nonzero width for a number of non-spacing combining
    # characters, e.g. (in 7.0.06): f84,17b4,17b5,180b,180d,2d7f, and
    # the variation selectors
    if cat==UTF8PROC_CATEGORY_MN
        CharWidths[c]=0
    end

    # We also assign width of one to unassigned and private-use
    # codepoints (Unifont includes ConScript Unicode Registry PUA fonts,
    # but since these are nonstandard it seems questionable to use Unifont metrics;
    # if they are printed as the replacement character U+FFFD they will have width 1).
    if cat==UTF8PROC_CATEGORY_CO || cat==UTF8PROC_CATEGORY_CN
        CharWidths[c]=1
    end

    # for some reason, Unifont has width-2 glyphs for ASCII control chars
    if cat==UTF8PROC_CATEGORY_CC
        CharWidths[c]=0
    end
end

#Soft hyphen is typically printed as a hyphen (-) in terminals.
CharWidths[0x00ad]=1

#By definition, should have zero width (on the same line)
#0x002028 ' ' category: Zl name: LINE SEPARATOR/
#0x002029 ' ' category: Zp name: PARAGRAPH SEPARATOR/
CharWidths[0x2028]=0
CharWidths[0x2029]=0

#############################################################################
# Output (to a file or pipe) for processing by data_generator.rb,
# encoded as a sequence of intervals.

firstc = 0x000000
lastv = 0
uhex(c) = uppercase(string(c,base=16,pad=4))
for c in 0x0000:0x110000
    global firstc, lastv
    v = get(CharWidths, c, 0)
    if v != lastv || c == 0x110000
        v < 4 || error("invalid charwidth $v for $c")
        if firstc+1 < c
            println(uhex(firstc), "..", uhex(c-1), "; ", lastv)
        else
            println(uhex(firstc), "; ", lastv)
        end
        firstc = c
        lastv = v
    end
end
