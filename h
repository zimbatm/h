#!/usr/bin/env ruby
#
# Usage: h <term>
#
# term can be of form [\w-]+ for search
# <user>/<repo> for github repos
# url for cloning

require 'find'
require 'json'
require 'open-uri'
require 'pathname'
require 'uri'

DEFAULT_CODE_ROOT = ENV['H_CODE_ROOT'] || '~/src'

def abort(*)
  puts Dir.pwd
  super
end

term = ARGV.shift
path = nil
url  = nil

case term
when "--setup"
  code_root = Pathname.new(ARGV[0] || DEFAULT_CODE_ROOT).expand_path
  puts <<-SH
h() {
  _h_dir=$(command h --resolve "#{code_root}" "$@")
  _h_ret=$?
  [ "$_h_dir" != "$PWD" ] && cd "$_h_dir"
  return $_h_ret
}
  SH
  exit
when "--resolve"
  CODE_ROOT = Pathname.new(ARGV.shift)
else
  puts "h is not installed"
  puts
  puts "h needs to be hooked into the shell before it can be used."
  puts

  abort "Usage: eval \"$(h --setup [code-root])\""
end

term = ARGV.shift
case term
when nil, "-h", "--help"
  abort "Usage: h (<name> | <repo>/<name> | <url>) [git opts]"
when %r[\A([\w\.\-]+)/([\w\.\-]+)\z] # github user/repo
  # query the github API to find out the right file case
  begin
    api_info = JSON.load(URI.open("https://api.github.com/repos/#{$1}/#{$2}").read)
    owner = api_info["owner"]["login"]
    repo = api_info["name"]
  rescue OpenURI::HTTPError
    owner = $1
    repo = $2
  end

  url  = "git@github.com:#{owner}/#{repo}.git"
  path = CODE_ROOT.join('github.com', owner, repo)
when %r[://] # URL
  url  = URI.parse(term)
  path = CODE_ROOT.join(url.host.downcase, url.path[1..-1])
  abort "Missing url scheme" unless url.scheme
when %r[\Agit(?:ea)?@([^:]+):(.*)] # git url
  url  = term
  path = CODE_ROOT.join($1, $2)
when %r[\A[\w\.\-]+\z] # just search for repo
  path_depth = 0

  if term == term.downcase then
    # case insentitive search
    compare = ->(basename) { basename.downcase == term }
  else
    compare = ->(basename) { basename == term }
  end

  # Find all matches
  CODE_ROOT.find do |curpath|
    next unless curpath.directory?

    depth = curpath.to_s.sub(CODE_ROOT.to_s, '').split('/').size

    # Select deepest result
    if compare.(curpath.basename.to_s) && depth > path_depth
      path = curpath
      path_depth = depth
    end

    # Don't search below 4
    Find.prune if depth > 3
  end
else
  abort "Unknown pattern for #{term}"
end

abort "#{term} not found" unless path

# Remove .git to path
path = path.sub_ext('') if path.extname == '.git'

unless path.directory?
  # Keep note of the existing path
  parent = path.parent
  dir = parent
  dir = dir.parent until dir.directory? || dir.root?
  # Make sure the parent directory exists
  parent.mkpath
  unless system(
      'git',
      'clone',
      *(ARGV.any? ? ARGV : ['--recursive']),
      '--',
      url.to_s,
      path.to_s,
      out: :err,
      close_others: true,
  )
    # Cleanup the parent directory if created
    until parent == dir
      parent.rmdir
      parent = parent.parent
    end

    exit $?.exitstatus
  end
end

puts path
