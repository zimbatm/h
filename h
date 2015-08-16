#!/usr/bin/env ruby
#
# Usage: h <term>
#
# term can be of form [\w-]+ for search
# <user>/<repo> for github repos
# url for cloning

require 'find'
require 'pathname'
require 'uri'

# TODO: make that configurable
CODE_ROOT = Pathname.new('~/code').expand_path

def abort(*)
  puts Dir.pwd
  super
end

USAGE = "Usage: h (--setup | <name> | <repo>/<name> | <url>) [git opts]"

term = ARGV.shift
path = nil
url  = nil

case term
when nil, "--help"
  abort USAGE
when "--setup"
  puts <<-SH
h() {
  _h_dir=$(command h "$@")
  _h_ret=$?
  [ "$_h_dir" != "$PWD" ] && cd "$_h_dir"
  return $_h_ret
}
  SH
  exit
when %r[\A([\w\.\-]+)/([\w\.\-]+)\z] # github user/repo
  url  = URI.parse("https://github.com/#{$1}/#{$2}.git")
  path = CODE_ROOT.join('github.com', $1, $2)
when %r[://] # URL
  url  = URI.parse(term)
  path = CODE_ROOT.join(url.host, url.path[1..-1])
  abort "Missing url scheme" unless url.scheme
when %r[\Agit@([^:]+):(.*)] # git url
  url  = term
  path = CODE_ROOT.join($1, $2)
when %r[\A[\w\.\-]+\z] # just search for repo
  path_depth = 0

  # Find all matches
  CODE_ROOT.find do |curpath|
    next unless curpath.directory?

    depth = curpath.to_s.sub(CODE_ROOT.to_s, '').split('/').size

    # Select deepest result
    if curpath.basename.to_s == term && depth > path_depth
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
      *(ARGV.any? ? ARGV : ['--recursive', '--depth', '1']),
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
