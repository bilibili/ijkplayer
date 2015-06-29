#!/usr/bin/env ruby

# This script fetches a pre-compiled copy of the iOS GHUnit.framework, if one isn't already in the External/GHUnit folder
# This replaces the old system, where GHUnit was included as a git submodule, because:
# a) git submodules confuse people (including me)
# b) GHUnit seems to be tricky to build without warnings
# The pre-compiled frameworks on allseeing-i.com were taken directly from those on the GHUnit downloads page on GitHub
# If you'd rather build GHUnit yourself, simply grab a copy from http://github.com/gabriel/gh-unit and drop your built framework into External/GHUnit

require 'net/http'
if (!File.exists?('External/GHUnit/GHUnitIOS.framework'))
	`curl -s http://allseeing-i.com/ASIHTTPRequest/GHUnit/GHUnit-IOS.zip > External/GHUnit/GHUnit-IOS.zip`
  	`unzip External/GHUnit/GHUnit-IOS.zip -d External/GHUnit/ & rm External/GHUnit/GHUnit-IOS.zip`
end