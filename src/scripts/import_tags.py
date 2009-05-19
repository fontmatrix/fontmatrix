# skeleton of an impot-tags-from-anywhere script !

# You need to fill in <other> yourself
# key:"absolute font path", value:[list of tags]
other = {}

db = Fontmatrix.DB()
for key, value in other[:]:
	for tag in value[:]:
		db.addTag(key, tag)

