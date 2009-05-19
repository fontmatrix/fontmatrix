#
# This script will tag your fonts and mark them activated/deactivated
# as appropriate according to your old Fontmatrix XML config.
#
# After running this you will need to use the Check Database feature of
# Fontmatrix to cleanup activated fonts marked for deactivation and vice
# versa.
#
# Up-to-date version at http://thomas-sibley.com/etc/tag-from-xml
#

from os import environ
from xml.dom import minidom

# Change this if your old XML config isn't in your home dir
xmlfile = environ["HOME"] + "/.fontmatrix.data"

def getText(element):
    rc = ""
    for node in element.childNodes:
        if node.nodeType == node.TEXT_NODE:
            rc = rc + node.data
    return rc

dom = minidom.parse(xmlfile).documentElement
db = Fontmatrix.DB();

for fontfile in dom.getElementsByTagName("fontfile"):
    file = getText(fontfile.getElementsByTagName("file")[0])
    font = db.Font(file);

    if font:
        for node in fontfile.getElementsByTagName("tag"):
            tag = getText(node)
            if tag == "Activated_On":
                font.Active = 1
            elif tag == "Activated_Off":
                font.Active = 0
            else:
                db.addTag(file, tag)
    else:
        print "Can't find " + file

Fontmatrix.updateTree();
