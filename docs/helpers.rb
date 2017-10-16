require 'bonsai'
require 'erb'
require 'json'
require 'liquid'
require 'maruku'
require 'tmpdir'
require 'yaml'

def load_manual
  YAML::load(File.open("content/3.manual/manual.yml"))
end

module ExtraFilters
  def markdownify(input)
    Maruku.new(input).to_html
  end
  def search_id(input)
    input.gsub(/`/, '')
  end
  def section_id(input)
    input.gsub(/[^a-zA-Z0-9_]/, '')
  end
  def entry_id(input)
    input.gsub(/[ `]/, '')
  end
  def no_paragraph(input)
    input.gsub('<p>', '').gsub('</p>', '')
  end
  def json(input)
    input.to_json
  end
  def unique(input)
    @n = (@n || 0) + 1
    input + @n.to_s
  end
  def dash_anchor(input)
    input.gsub /`/, ''
  end
  def percent_encode(input)
    ERB::Util.url_encode(input)
  end
end

Liquid::Template.register_filter(ExtraFilters)

begin
  `java 2>&1`
rescue
  class Bonsai::Exporter
    def self.compress_assets
      Bonsai.log "java not found! Not compressing javascript or stylesheets"
    end
  end
end

