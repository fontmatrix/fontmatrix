# Subset the current font according to a reference text

# Paste your text here
# All letters composing the text will be kept in the resulting font
Text = unicode("TEST")

import os.path
import sys
# Change it according to your own installation of fontforge python module
sys.path.append("/home/pierre/system/lib/python2.6/site-packages/")

try:
	import fontforge
except importError:
	print "failed to import fontforge module"

cf = Fontmatrix.currentFontPath()
ff = fontforge
font1 = ff.open(cf)
RemList = []

for uc in font1.glyphs():
	if(uc.unicode >= 0):
		if(False == (unichr(uc.unicode) in Text)):
			RemList.append(uc)
		else:
			print unicode(uc.glyphname) + u" is in the Text as " + unichr(uc.unicode)
	else:
		print uc.glyphname + " has codepoint " + uc.unicode
		
for r in RemList:
	font1.removeGlyph(r)

resname = "subseted-"+ os.path.basename(cf)
try:
	font1.generate(resname)
except EnvironmentError:
	print "Something went wrong when generating: "+resname

font1.close()