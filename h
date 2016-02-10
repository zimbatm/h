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

def abort(*)
  puts Dir.pwd
  super
end

term = ARGV.shift
path = nil
url  = nil

case term
when nil, "-h", "--help"
  puts "h is not installed"
  puts

  abort "Usage: eval $(h --setup [code-root])"
when "--setup"
  code_root = Pathname.new(ARGV[0] || '~/code').expand_path
  puts <<-SH
h() {
  _h_dir=$(command h "#{code_root}" "$@")
  _h_ret=$?
  [ "$_h_dir" != "$PWD" ] && cd "$_h_dir"
  return $_h_ret
}
  SH
  exit
else
  CODE_ROOT = Pathname.new(term)
end

term = ARGV.shift
case term
when nil, "-h", "--help"
  abort "Usage: h (<name> | <repo>/<name> | <url>) [git opts]"
when %r[\A([\w\.\-]+)/([\w\.\-]+)\z] # github user/repo
  url  = "git@github.com:#{$1}/#{$2}.git"
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
