BEGIN{
	FS=";"
	modele="p[ bKey( 0xSTART , 0xEND) ] = QObject::tr(\"BLOCK\") ;"
}
{
	m = modele
	if(!($1 ~ /#/) && (length($0) > 0))
	{
		# we want to split XXXXX..XXXXX
		split($1, StartAndEnd,".")
		start = StartAndEnd[1]
		end = StartAndEnd[3]
		sub(/START/, start , m)
		sub(/END/, end , m)
		
		block = $2
		sub(/^ */,"",block)
    	sub(/ *$/,"",block) 
		sub(/BLOCK/, block, m)
		print m > "uniblocks.cxx"
# 		print m
	}

}