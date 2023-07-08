# Unicode data generation rules.  Except for the test data files, most
# users will not use these Makefile rules, which are primarily to re-generate
# unicode_data.c when we get a new Unicode version or charwidth data; they
# require ruby and julia to be installed.

# programs
CURL=curl
RUBY=ruby
PERL=perl
MAKE=make
JULIA=julia
CURLFLAGS = --retry 5 --location

.PHONY: clean

.DELETE_ON_ERROR:

utf8proc_data.c.new: data_generator.rb UnicodeData.txt GraphemeBreakProperty.txt DerivedCoreProperties.txt CompositionExclusions.txt CaseFolding.txt CharWidths.txt emoji-data.txt
	$(RUBY) data_generator.rb < UnicodeData.txt > $@

CharWidths.txt: charwidths.jl EastAsianWidth.txt
	$(JULIA) charwidths.jl > $@

# Unicode data version (must also update utf8proc_unicode_version function)
UNICODE_VERSION=15.0.0

UnicodeData.txt:
	$(CURL) $(CURLFLAGS) -o $@ https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/UnicodeData.txt

EastAsianWidth.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/EastAsianWidth.txt

GraphemeBreakProperty.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/auxiliary/GraphemeBreakProperty.txt

DerivedCoreProperties.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/DerivedCoreProperties.txt

CompositionExclusions.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/CompositionExclusions.txt

CaseFolding.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/CaseFolding.txt

NormalizationTest.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/NormalizationTest.txt

GraphemeBreakTest.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://www.unicode.org/Public/$(UNICODE_VERSION)/ucd/auxiliary/GraphemeBreakTest.txt

emoji-data.txt:
	$(CURL) $(CURLFLAGS) -o $@ $(URLCACHE)https://unicode.org/Public/$(UNICODE_VERSION)/ucd/emoji/emoji-data.txt

Uppercase.txt: DerivedCoreProperties.txt
	$(RUBY) -e 'puts File.read("DerivedCoreProperties.txt")[/# Derived Property: Uppercase.*?# Total code points:/m]' > $@

Lowercase.txt: DerivedCoreProperties.txt
	$(RUBY) -e 'puts File.read("DerivedCoreProperties.txt")[/# Derived Property: Lowercase.*?# Total code points:/m]' > $@

clean:
	rm -f UnicodeData.txt EastAsianWidth.txt GraphemeBreakProperty.txt DerivedCoreProperties.txt CompositionExclusions.txt CaseFolding.txt NormalizationTest.txt GraphemeBreakTest.txt CharWidths.txt emoji-data.txt
	rm -f Uppercase.txt Lowercase.txt
	rm -f utf8proc_data.c.new
