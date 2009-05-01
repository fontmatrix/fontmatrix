import os
import tempfile

try:
	from fontTools import ttLib
except ImportError:
	print "cannot import ttLib"


tfH, tfP = tempfile.mkstemp()
tt = ttLib.TTFont(Fontmatrix.currentFontPath())
tt.saveXML(tfP);

ret = ""
cont = True
while (cont):
	tret = os.read(tfH, 256)
	if(len(tret) == 0):
		cont = False
	ret += tret
os.close(tfH)

print ret