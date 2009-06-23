BEGIN{
	FS=";"
	i = 1
	j = 1
	fr[0] = "NONE"
	while((getline line < "Blocks-fr.txt") > 0)	
	{
		split(line, top, ";")
		
		block = top[2]
		sub(/^ */,"",block)
    	sub(/ *$/,"",block) 
		fr[i] = block
		i = i + 1
	}
	close("Blocks-fr.txt")
}
{
	if(!($1 ~ /#/) && (length($0) > 0))
	{
		block = $2
		sub(/^ */,"",block)
    	sub(/ *$/,"",block)
		
		print "<message><location filename=\"../langs/unicode/uniblocks.cxx\" line=\"" j "\"/><source>"block"</source><translation>"fr[j]"</translation> </message>"
		j = j + 1
	}

}