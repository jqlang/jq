#!/usr/bin/env ruby

#  This file was used to generate the 'unicode_data.c' file by parsing the
#  Unicode data file 'UnicodeData.txt' of the Unicode Character Database.
#  It is included for informational purposes only and not intended for
#  production use.


#  Copyright (c) 2018 Steven G. Johnson, Tony Kelman, Keno Fischer,
#                Benito van der Zander, Michaël Meyer, and other contributors.
#  Copyright (c) 2009 Public Software Group e. V., Berlin, Germany
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.


#  This file contains derived data from a modified version of the
#  Unicode data files. The following license applies to that data:
#
#  COPYRIGHT AND PERMISSION NOTICE
#
#  Copyright (c) 1991-2007 Unicode, Inc. All rights reserved. Distributed
#  under the Terms of Use in http://www.unicode.org/copyright.html.
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of the Unicode data files and any associated documentation (the "Data
#  Files") or Unicode software and any associated documentation (the
#  "Software") to deal in the Data Files or Software without restriction,
#  including without limitation the rights to use, copy, modify, merge,
#  publish, distribute, and/or sell copies of the Data Files or Software, and
#  to permit persons to whom the Data Files or Software are furnished to do
#  so, provided that (a) the above copyright notice(s) and this permission
#  notice appear with all copies of the Data Files or Software, (b) both the
#  above copyright notice(s) and this permission notice appear in associated
#  documentation, and (c) there is clear notice in each modified Data File or
#  in the Software as well as in the documentation associated with the Data
#  File(s) or Software that the data or software has been modified.
#
#  THE DATA FILES AND SOFTWARE ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
#  KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
#  THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
#  INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR
#  CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
#  USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
#  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
#  PERFORMANCE OF THE DATA FILES OR SOFTWARE.
#
#  Except as contained in this notice, the name of a copyright holder shall
#  not be used in advertising or otherwise to promote the sale, use or other
#  dealings in these Data Files or Software without prior written
#  authorization of the copyright holder.


$ignorable_list = File.read("DerivedCoreProperties.txt", :encoding => 'utf-8')[/# Derived Property: Default_Ignorable_Code_Point.*?# Total code points:/m]
$ignorable = []
$ignorable_list.each_line do |entry|
  if entry =~ /^([0-9A-F]+)\.\.([0-9A-F]+)/
    $1.hex.upto($2.hex) { |e2| $ignorable << e2 }
  elsif entry =~ /^[0-9A-F]+/
    $ignorable << $&.hex
  end
end

$uppercase_list = File.read("DerivedCoreProperties.txt", :encoding => 'utf-8')[/# Derived Property: Uppercase.*?# Total code points:/m]
$uppercase = []
$uppercase_list.each_line do |entry|
  if entry =~ /^([0-9A-F]+)\.\.([0-9A-F]+)/
    $1.hex.upto($2.hex) { |e2| $uppercase << e2 }
  elsif entry =~ /^[0-9A-F]+/
    $uppercase << $&.hex
  end
end

$lowercase_list = File.read("DerivedCoreProperties.txt", :encoding => 'utf-8')[/# Derived Property: Lowercase.*?# Total code points:/m]
$lowercase = []
$lowercase_list.each_line do |entry|
  if entry =~ /^([0-9A-F]+)\.\.([0-9A-F]+)/
    $1.hex.upto($2.hex) { |e2| $lowercase << e2 }
  elsif entry =~ /^[0-9A-F]+/
    $lowercase << $&.hex
  end
end

$grapheme_boundclass_list = File.read("GraphemeBreakProperty.txt", :encoding => 'utf-8')
$grapheme_boundclass = Hash.new("UTF8PROC_BOUNDCLASS_OTHER")
$grapheme_boundclass_list.each_line do |entry|
  if entry =~ /^([0-9A-F]+)\.\.([0-9A-F]+)\s*;\s*([A-Za-z_]+)/
    $1.hex.upto($2.hex) { |e2| $grapheme_boundclass[e2] = "UTF8PROC_BOUNDCLASS_" + $3.upcase }
  elsif entry =~ /^([0-9A-F]+)\s*;\s*([A-Za-z_]+)/
    $grapheme_boundclass[$1.hex] = "UTF8PROC_BOUNDCLASS_" + $2.upcase
  end
end

$emoji_data_list = File.read("emoji-data.txt", :encoding => 'utf-8')
$emoji_data_list.each_line do |entry|
  if entry =~ /^([0-9A-F]+)\.\.([0-9A-F]+)\s*;\s*Extended_Pictographic\W/
    $1.hex.upto($2.hex) { |e2| $grapheme_boundclass[e2] = "UTF8PROC_BOUNDCLASS_EXTENDED_PICTOGRAPHIC" }
  elsif entry =~ /^([0-9A-F]+)\s*;\s*Extended_Pictographic\W/
    $grapheme_boundclass[$1.hex] = "UTF8PROC_BOUNDCLASS_EXTENDED_PICTOGRAPHIC"
  elsif entry =~ /^([0-9A-F]+)\.\.([0-9A-F]+)\s*;\s*Emoji_Modifier\W/
    $1.hex.upto($2.hex) { |e2| $grapheme_boundclass[e2] = "UTF8PROC_BOUNDCLASS_EXTEND" }
  elsif entry =~ /^([0-9A-F]+)\s*;\s*Emoji_Modifier\W/
    $grapheme_boundclass[$1.hex] = "UTF8PROC_BOUNDCLASS_EXTEND"
  end
end

$charwidth_list = File.read("CharWidths.txt", :encoding => 'utf-8')
$charwidth = Hash.new(0)
$charwidth_list.each_line do |entry|
  if entry =~ /^([0-9A-F]+)\.\.([0-9A-F]+)\s*;\s*([0-9]+)/
    $1.hex.upto($2.hex) { |e2| $charwidth[e2] = $3.to_i }
  elsif entry =~ /^([0-9A-F]+)\s*;\s*([0-9]+)/
    $charwidth[$1.hex] = $2.to_i
  end
end

$exclusions = File.read("CompositionExclusions.txt", :encoding => 'utf-8')[/# \(1\) Script Specifics.*?# Total code points:/m]
$exclusions = $exclusions.chomp.split("\n").collect { |e| e.hex }

$excl_version = File.read("CompositionExclusions.txt", :encoding => 'utf-8')[/# \(2\) Post Composition Version precomposed characters.*?# Total code points:/m]
$excl_version = $excl_version.chomp.split("\n").collect { |e| e.hex }

$case_folding_string = File.read("CaseFolding.txt", :encoding => 'utf-8')
$case_folding = {}
$case_folding_string.chomp.split("\n").each do |line|
  next unless line =~ /([0-9A-F]+); [CF]; ([0-9A-F ]+);/i
  $case_folding[$1.hex] = $2.split(" ").collect { |e| e.hex }
end

$int_array = []
$int_array_indicies = {}

def str2c(string, prefix)
  return "0" if string.nil?
  return "UTF8PROC_#{prefix}_#{string.upcase}"
end
def pushary(array)
  idx = $int_array_indicies[array]
  unless idx
    $int_array_indicies[array] = $int_array.length
    idx = $int_array.length
    array.each { |entry| $int_array << entry }
  end
  return idx
end
def cpary2utf16encoded(array)
  return array.flat_map { |cp|
      if (cp <= 0xFFFF)
        raise "utf-16 code: #{cp}" if cp & 0b1111100000000000 == 0b1101100000000000
        cp
      else
        temp = cp - 0x10000
        [(temp >> 10) | 0b1101100000000000, (temp & 0b0000001111111111) | 0b1101110000000000]
      end
    }
end
def cpary2c(array)
  return "UINT16_MAX" if array.nil? || array.length == 0
  lencode = array.length - 1 #no sequence has len 0, so we encode len 1 as 0, len 2 as 1, ...
  array = cpary2utf16encoded(array)
  if lencode >= 3 #we have only 2 bits for the length 
    array = [lencode] + array
    lencode = 3
  end
  idx = pushary(array)
  raise "Array index out of bound" if idx > 0x3FFF
  return "#{idx | (lencode << 14)}"
end
def singlecpmap(cp)
  return "UINT16_MAX" if cp == nil
  idx = pushary(cpary2utf16encoded([cp]))
  raise "Array index out of bound" if idx > 0xFFFF
  return "#{idx}"
end

class UnicodeChar
  attr_accessor :code, :name, :category, :combining_class, :bidi_class,
                :decomp_type, :decomp_mapping,
                :bidi_mirrored,
                :uppercase_mapping, :lowercase_mapping, :titlecase_mapping,
                #caches:
                :c_entry_index, :c_decomp_mapping, :c_case_folding
  def initialize(line)
    raise "Could not parse input." unless line =~ /^
      ([0-9A-F]+);        # code
      ([^;]+);            # name
      ([A-Z]+);           # general category
      ([0-9]+);           # canonical combining class
      ([A-Z]+);           # bidi class
      (<([A-Z]*)>)?       # decomposition type
      ((\ ?[0-9A-F]+)*);  # decompomposition mapping
      ([0-9]*);           # decimal digit
      ([0-9]*);           # digit
      ([^;]*);            # numeric
      ([YN]*);            # bidi mirrored
      ([^;]*);            # unicode 1.0 name
      ([^;]*);            # iso comment
      ([0-9A-F]*);        # simple uppercase mapping
      ([0-9A-F]*);        # simple lowercase mapping
      ([0-9A-F]*)$/ix     # simple titlecase mapping
    @code              = $1.hex
    @name              = $2
    @category          = $3
    @combining_class   = Integer($4)
    @bidi_class        = $5
    @decomp_type       = $7
    @decomp_mapping    = ($8=='') ? nil :
                         $8.split.collect { |element| element.hex }
    @bidi_mirrored     = ($13=='Y') ? true : false
    # issue #130: use nonstandard uppercase ß -> ẞ
    # issue #195: if character is uppercase but has no lowercase mapping,
    #             then make lowercase mapping = itself (vice versa for lowercase)
    @uppercase_mapping = ($16=='') ? (code==0x00df ? 0x1e9e : ($17=='' && $lowercase.include?(code) ? code : nil)) : $16.hex
    @lowercase_mapping = ($17=='') ? ($16=='' && $uppercase.include?(code) ? code : nil) : $17.hex
    @titlecase_mapping = ($18=='') ? (code==0x00df ? 0x1e9e : nil) : $18.hex
  end
  def case_folding
    $case_folding[code]
  end
  def c_entry(comb_indicies)
    "  " <<
    "{#{str2c category, 'CATEGORY'}, #{combining_class}, " <<
    "#{str2c bidi_class, 'BIDI_CLASS'}, " <<
    "#{str2c decomp_type, 'DECOMP_TYPE'}, " <<
    "#{c_decomp_mapping}, " <<
    "#{c_case_folding}, " <<
    "#{singlecpmap uppercase_mapping }, " <<
    "#{singlecpmap lowercase_mapping }, " <<
    "#{singlecpmap titlecase_mapping }, " <<
    "#{comb_indicies[code] ? comb_indicies[code]: 'UINT16_MAX'}, " <<
    "#{bidi_mirrored}, " <<
    "#{$exclusions.include?(code) or $excl_version.include?(code)}, " <<
    "#{$ignorable.include?(code)}, " <<
    "#{%W[Zl Zp Cc Cf].include?(category) and not [0x200C, 0x200D].include?(category)}, " <<
    "#{$charwidth[code]}, 0, " <<
    "#{$grapheme_boundclass[code]}},\n"
  end
end

chars = []
char_hash = {}

while gets
  if $_ =~ /^([0-9A-F]+);<[^;>,]+, First>;/i
    first = $1.hex
    gets
    char = UnicodeChar.new($_)
    raise "No last character of sequence found." unless
      $_ =~ /^([0-9A-F]+);<([^;>,]+), Last>;/i
    last = $1.hex
    name = "<#{$2}>"
    for i in first..last
      char_clone = char.clone
      char_clone.code = i
      char_clone.name = name
      char_hash[char_clone.code] = char_clone
      chars << char_clone
    end
  else
    char = UnicodeChar.new($_)
    char_hash[char.code] = char
    chars << char
  end
end

comb1st_indicies = {}
comb2nd_indicies = {}
comb2nd_indicies_sorted_keys = []
comb2nd_indicies_nonbasic = {}
comb_array = []

chars.each do |char|
  if !char.nil? and char.decomp_type.nil? and char.decomp_mapping and
      char.decomp_mapping.length == 2 and !char_hash[char.decomp_mapping[0]].nil? and
      char_hash[char.decomp_mapping[0]].combining_class == 0 and
      not $exclusions.include?(char.code)

    dm0 = char.decomp_mapping[0]
    dm1 = char.decomp_mapping[1]
    unless comb1st_indicies[dm0]
      comb1st_indicies[dm0] = comb1st_indicies.keys.length
    end
    unless comb2nd_indicies[dm1]
      comb2nd_indicies_sorted_keys << dm1
      comb2nd_indicies[dm1] = comb2nd_indicies.keys.length
    end
    comb_array[comb1st_indicies[dm0]] ||= []
    raise "Duplicate canonical mapping: #{char.code} #{dm0} #{dm1}" if comb_array[comb1st_indicies[dm0]][comb2nd_indicies[dm1]]
    comb_array[comb1st_indicies[dm0]][comb2nd_indicies[dm1]] = char.code

    comb2nd_indicies_nonbasic[dm1] = true if char.code > 0xFFFF
  end
  char.c_decomp_mapping = cpary2c(char.decomp_mapping)
  char.c_case_folding = cpary2c(char.case_folding)
end

comb_indicies = {}
cumoffset = 0
comb1st_indicies_lastoffsets = []
comb1st_indicies_firstoffsets = []
comb1st_indicies.each do |dm0, index|
  first = nil
  last = nil
  offset = 0
  comb2nd_indicies_sorted_keys.each_with_index do |dm1, b|
    if comb_array[index][b]
      first = offset unless first
      last = offset
      last += 1 if comb2nd_indicies_nonbasic[dm1]
    end
    offset += 1
    offset += 1 if comb2nd_indicies_nonbasic[dm1]
  end
  comb1st_indicies_firstoffsets[index] = first
  comb1st_indicies_lastoffsets[index] = last
  raise "double index" if comb_indicies[dm0]
  comb_indicies[dm0] = cumoffset
  cumoffset += last - first + 1 + 2
end

offset = 0
comb2nd_indicies_sorted_keys.each do |dm1|
  raise "double index" if comb_indicies[dm1]
  comb_indicies[dm1] = 0x8000 | (comb2nd_indicies[dm1] + offset)
  raise "too large comb index" if  comb2nd_indicies[dm1] + offset > 0x4000
  if comb2nd_indicies_nonbasic[dm1]
    comb_indicies[dm1] = comb_indicies[dm1] | 0x4000
    offset += 1
  end
end

properties_indicies = {}
properties = []
chars.each do |char|
  c_entry = char.c_entry(comb_indicies)
  char.c_entry_index = properties_indicies[c_entry]
  unless char.c_entry_index
    properties_indicies[c_entry] = properties.length
    char.c_entry_index = properties.length
    properties << c_entry
  end
end

stage1 = []
stage2 = []
for code in 0...0x110000
  next unless code % 0x100 == 0
  stage2_entry = []
  for code2 in code...(code+0x100)
    if char_hash[code2]
      stage2_entry << (char_hash[code2].c_entry_index + 1)
    else
      stage2_entry << 0
    end
  end
  old_index = stage2.index(stage2_entry)
  if old_index
    stage1 << (old_index * 0x100)
  else
    stage1 << (stage2.length * 0x100)
    stage2 << stage2_entry
  end
end

$stdout << "static const utf8proc_uint16_t utf8proc_sequences[] = {\n  "
i = 0
$int_array.each do |entry|
  i += 1
  if i == 8
    i = 0
    $stdout << "\n  "
  end
  $stdout << entry << ", "
end
$stdout << "};\n\n"

$stdout << "static const utf8proc_uint16_t utf8proc_stage1table[] = {\n  "
i = 0
stage1.each do |entry|
  i += 1
  if i == 8
    i = 0
    $stdout << "\n  "
  end
  $stdout << entry << ", "
end
$stdout << "};\n\n"

$stdout << "static const utf8proc_uint16_t utf8proc_stage2table[] = {\n  "
i = 0
stage2.flatten.each do |entry|
  i += 1
  if i == 8
    i = 0
    $stdout << "\n  "
  end
  $stdout << entry << ", "
end
$stdout << "};\n\n"

$stdout << "static const utf8proc_property_t utf8proc_properties[] = {\n"
$stdout << "  {0, 0, 0, 0, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX,  false,false,false,false, 1, 0, UTF8PROC_BOUNDCLASS_OTHER},\n"
properties.each { |line|
  $stdout << line
}
$stdout << "};\n\n"



$stdout << "static const utf8proc_uint16_t utf8proc_combinations[] = {\n  "
i = 0
comb1st_indicies.keys.each_index do |a|
  offset = 0
  $stdout << comb1st_indicies_firstoffsets[a] << ", " << comb1st_indicies_lastoffsets[a] << ", "
  comb2nd_indicies_sorted_keys.each_with_index do |dm1, b|
    break if offset > comb1st_indicies_lastoffsets[a]
    if offset >= comb1st_indicies_firstoffsets[a]
      i += 1
      if i == 8
        i = 0
        $stdout << "\n  "
      end
      v = comb_array[a][b] ? comb_array[a][b] : 0
      $stdout << (( v & 0xFFFF0000 ) >> 16) << ", " if comb2nd_indicies_nonbasic[dm1]
      $stdout << (v & 0xFFFF) << ", "
    end
    offset += 1
    offset += 1 if comb2nd_indicies_nonbasic[dm1]
  end
  $stdout  << "\n"
end
$stdout << "};\n\n"
