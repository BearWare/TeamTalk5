
genstrings:
	genstrings -o Base.lproj/ *.swift
	genstrings -o en.lproj/ *.swift
	iconv -f UTF-16LE -t UTF-8 Base.lproj/Localizable.strings > Base.lproj/Localizable.strings.utf8
	mv Base.lproj/Localizable.strings.utf8 Base.lproj/Localizable.strings
	iconv -f UTF-16LE -t UTF-8 en.lproj/Localizable.strings > en.lproj/Localizable.strings.utf8
	mv en.lproj/Localizable.strings.utf8 en.lproj/Localizable.strings
	make TARGET=Base.lproj/Localizable.strings replace
	make TARGET=en.lproj/Localizable.strings replace

# replace %2$d %1$@
replace:
	sed -i "" -e 's/%[0-9]\$$d/%d/g' $(TARGET)
	sed -i "" -e 's/%[0-9]\$$@/%@/g' $(TARGET)
