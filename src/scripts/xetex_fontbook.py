# Type your script right here
cf = Fontmatrix.currentFonts()
head = "\\hsize=360pt \n\\vsize=580pt"
head += "\\baselineskip=14.5pt \n\\parindent=8pt \n\\frenchspacing  \n\\XeTeXlinebreaklocale fr \n\\overfullrule=6pt \n\n"
body=""
for f in cf[:]:
	body += "\n"
	body += "\\font\\lafonte=\"[" + f.Path + "] at 11pt"
	body += "\n";
	body += "\\lafonte";
	body += "\n";
	body += f.Family + " " + f.Variant
	body += "\n";
	
print head
print body

