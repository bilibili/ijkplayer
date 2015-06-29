#!/usr/bin/env ruby

# This script sets a version number for ASIHTTPRequest based on the last commit, and is run when you build one of the targets in the Xcode projects that come with ASIHTTPRequest
# It only really needs to run on my computer, not on yours... :)
require 'find'
if (File.exists?('.git') && File.directory?('.git') && File.exists?('/usr/local/bin/git'))
	newversion = `/usr/local/bin/git describe --tags`.match(/(v([0-9]+)(\.([0-9]+)){1,}-([0-9]+))/).to_s.gsub(/[0-9]+$/){|commit| (commit.to_i + 1).to_s}+Time.now.strftime(" %Y-%m-%d")
	buffer = File.new('Classes/ASIHTTPRequest.m','r').read
	if !buffer.match(/#{Regexp.quote(newversion)}/)
		buffer = buffer.sub(/(NSString \*ASIHTTPRequestVersion = @\")(.*)(";)/,'\1'+newversion+'\3');
		File.open('Classes/ASIHTTPRequest.m','w') {|fw| fw.write(buffer)}
	end
end