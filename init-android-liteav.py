#! /usr/bin/env python
import os
import fnmatch
import fileinput
import shutil
import errno

__doc__ = '''

	./init-android.sh
	cd android/contrib
	./compile-ffmpeg.sh all
	cd ../..
	./init-android-liteav.py
	cd android
	./compile-ijk.sh all

'''
print "== change package name"

def get_filepaths(directory):
	"""
	This function will generate the file names in a directory 
	tree by walking the tree either top-down or bottom-up. For each 
	directory in the tree rooted at directory top (including top itself), 
	it yields a 3-tuple (dirpath, dirnames, filenames).
	"""
	file_paths = []  # List which will store all of the full filepaths.

	# Walk the tree.
	for root, directories, files in os.walk(directory):
		for filename in files:
			# Join the two strings in order to form the full filepath.
			filepath = os.path.join(root, filename)
			file_paths.append(filepath)  # Add it to the list.

	return file_paths  # Self-explanatory.

# Run the above function and store its results in a variable.   
paths = get_filepaths("./ijkmedia")
paths = paths + get_filepaths("./android/ijkplayer")
paths = paths + get_filepaths("./android/patches")
for fp in paths:
	x = fileinput.input(fp, inplace=True)
	for line in x:
		line = line.replace("tv.danmaku.ijk", "com.tencent.liteav")
		line = line.replace("tv_danmaku_ijk", "com_tencent_liteav")
		line = line.replace("tv/danmaku/ijk", "com/tencent/liteav")
		print line,
	x.close()

print "== rename folder"
def mv_target(root):
	os.system("mkdir -p {0}/com/tencent/liteav".format(root))
	os.system("rm -rf {0}/com/tencent/liteav/*".format(root))
	os.system("cp -rf {0}/tv/danmaku/ijk/ {0}/com/tencent/liteav".format(root))
	os.system("rm -rf {0}/tv".format(root))

mv_target("ijkmedia/ijkj4a/j4a/class")
mv_target("ijkmedia/ijkj4a/java")
mv_target("android/ijkplayer/ijkplayer-armv5/src/main/java")
mv_target("android/ijkplayer/ijkplayer-armv7a/src/main/java")
mv_target("android/ijkplayer/ijkplayer-arm64/src/main/java")
mv_target("android/ijkplayer/ijkplayer-x86/src/main/java")
# mv_target("android/ijkplayer/ijkplayer-x86_64/src/main/java")
mv_target("android/ijkplayer/ijkplayer-exo/src/main/java")
mv_target("android/ijkplayer/ijkplayer-java/src/main/java")


