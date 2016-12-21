
genstrings:
		genstrings -o en.lproj/ *.swift
	make TARGET=en.lproj/Localizable.strings iconv
		make TARGET=en.lproj/Localizable.strings replace

iconv:
	iconv -f UTF-16LE -t UTF-8 $(TARGET) > $(TARGET).utf8
	mv $(TARGET).utf8 $(TARGET)

# replace %2$d %1$@
replace:
	sed -i "" -e 's/%[0-9]\$$d/%d/g' $(TARGET)
	sed -i "" -e 's/%[0-9]\$$@/%@/g' $(TARGET)
