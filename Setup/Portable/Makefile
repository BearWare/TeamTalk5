VERSION="v5.8.2.5049"

OUTPUT_DEST=./

TT5SVN=https://github.com/BearWare/TeamTalk5/trunk

first:
	echo "No target"

archive: arcdeb32 arcdeb64 arcclassic arcrasp arccentos5 arccentos7 #arcmac arcwin32 arcwin64 

arctar:
	rm -f $(TT5DIST).tgz
	tar czf $(TT5DIST).tgz $(TT5DIST)
	make TARGET=$(TT5DIST).tgz scp

arczip:
	rm -f $(TT5DIST).zip
	7z a -tzip $(TT5DIST).zip $(TT5DIST)
	make TARGET=$(TT5DIST).zip scp

scp:
	true

win: win32 win64 classic

win32:
	$(eval TT5DIST=teamtalk-$(VERSION)-win32)
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	$(eval SRVNAME=$(TEAMTALK_ROOT)/Server/tt5srv.exe)
	$(eval SVCNAME=$(TEAMTALK_ROOT)/Server/tt5svc.exe)
	$(eval CLIENTEXE=$(TEAMTALK_ROOT)/Client/qtTeamTalk/TeamTalk.exe)
	$(eval CLIENTDLL=$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/TeamTalk5.dll)
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) license winhelp
	make OUTDIR=$(OUTDIR) SRVNAME=$(SRVNAME) winserver
	make OUTDIR=$(OUTDIR) SVCNAME=$(SVCNAME) winservice
	make OUTDIR=$(OUTDIR) CLIENTEXE=$(CLIENTEXE) CLIENTDLL=$(CLIENTDLL) winclient
	make arczip

win64:
	$(eval TT5DIST=teamtalk-$(VERSION)-win64)
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	$(eval SRVNAME=$(TEAMTALK_ROOT)/Server/tt5srv.exe)
	$(eval SVCNAME=$(TEAMTALK_ROOT)/Server/tt5svc.exe)
	$(eval CLIENTEXE=$(TEAMTALK_ROOT)/Client/qtTeamTalk/TeamTalk.exe)
	$(eval CLIENTDLL=$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/TeamTalk5.dll)
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) license winhelp
	make OUTDIR=$(OUTDIR) SRVNAME=$(SRVNAME) winserver
	make OUTDIR=$(OUTDIR) SVCNAME=$(SVCNAME) winservice
	make OUTDIR=$(OUTDIR) CLIENTEXE=$(CLIENTEXE) CLIENTDLL=$(CLIENTDLL) winclient
	make arczip

portable:
	$(eval TT5DIST=TeamTalk_$(VERSION)_Portable)
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	$(eval CLIENTEXE=$(TEAMTALK_ROOT)/Client/TeamTalkClassic/TeamTalk5Classic.exe)
	$(eval CLIENTDLL=$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/TeamTalk5.dll)
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) license classichelp
	cp $(CLIENTEXE) $(OUTDIR)/
	cp $(CLIENTDLL) $(OUTDIR)/
	cp -r $(TEAMTALK_ROOT)/Setup/Client/Sounds $(OUTDIR)/Sounds
	cp -r $(TEAMTALK_ROOT)/Setup/Client/Windows/TeamTalk5Classic.xml $(OUTDIR)/
	cp $(TEAMTALK_ROOT)/Setup/Client/Windows/TeamTalk5Classic.xml $(OUTDIR)/TeamTalk5Classic.xml.default
	cp -r $(TEAMTALK_ROOT)/Client/TeamTalkClassic/lng_classic $(OUTDIR)/lng_classic
	cp -r $(OUTPUT_DEST)/accessibility/tolk/x86/* $(OUTDIR)/
	# Copy Qt client
	cp $(TEAMTALK_ROOT)/Client/qtTeamTalk/TeamTalk.exe $(OUTDIR)/
	cp $(TEAMTALK_ROOT)/Setup/Client/Windows/TeamTalk5.ini $(OUTDIR)/TeamTalk5.ini.default
	mkdir -p $(OUTDIR)/languages
	cp $(TEAMTALK_ROOT)/Client/qtTeamTalk/languages/*.qm $(OUTDIR)/languages/
	make TT5DIST=$(TT5DIST) arczip

win32pro:
	$(eval TT5DIST=teamtalkpro-$(VERSION)-win32)
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	$(eval SRVNAME="$(TEAMTALK_ROOT)/Server/tt5prosrv.exe $(TEAMTALK_ROOT)/Server/tt5prosvc.exe")
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) SRVNAME=$(SRVNAME) winserverpro
	make TT5DIST=$(TT5DIST) arczip

deb32:
	make TT5DIST=teamtalk-$(VERSION)-debian9-i386 linux

deb64:
	make TT5DIST=teamtalk-$(VERSION)-debian9-x86_64 linux

ubuntu18:
	make TT5DIST=teamtalk-$(VERSION)-ubuntu18-x86_64 linux

rasp:
	make TT5DIST=teamtalk-$(VERSION)-raspbian10-armhf linux

linux:
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	$(eval SRVNAME=$(TEAMTALK_ROOT)/Server/tt5srv)
	$(eval CLIENTEXE=$(TEAMTALK_ROOT)/Client/qtTeamTalk/teamtalk)
	$(eval CLIENTDLL=$(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5.so)
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) license
	make OUTDIR=$(OUTDIR) SRVNAME=$(SRVNAME) server
	make OUTDIR=$(OUTDIR) CLIENTEXE=$(CLIENTEXE) CLIENTDLL=$(CLIENTDLL) debclient
	make SRCDIR=$(TEAMTALK_ROOT) OUTDIR=$(OUTDIR) deblang debhelp
	make arctar

linuxpro:
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	$(eval SRVNAME=$(TEAMTALK_ROOT)/Server/tt5prosrv)
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) SRVNAME=$(SRVNAME) serverpro
	make arctar

deb32pro:
	$(MAKE) TT5DIST=teamtalkpro-$(VERSION)-debian9-i386 linuxpro

deb64pro:
	$(MAKE) TT5DIST=teamtalkpro-$(VERSION)-debian9-x86_64 linuxpro

centos7pro:
	$(MAKE) TT5DIST=teamtalkpro-$(VERSION)-centos7-x86_64 linuxpro

ubuntu18pro:
	$(MAKE) TT5DIST=teamtalkpro-$(VERSION)-ubuntu18-x86_64 linuxpro

rasppro:
	$(MAKE) TT5DIST=teamtalkpro-$(VERSION)-raspbian10-armhf linuxpro

centos5:
	$(eval TT5DIST=teamtalk-$(VERSION)-centos5-i386)
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	$(eval SRVNAME=$(TEAMTALK_ROOT)/Server/tt5srv)
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) license
	make OUTDIR=$(OUTDIR) SRVNAME=$(SRVNAME) server
	make arctar

centos7:
	$(eval TT5DIST=teamtalk-$(VERSION)-centos7-x86_64)
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	$(eval SRVNAME=$(TEAMTALK_ROOT)/Server/tt5srv)
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	$(MAKE) OUTDIR=$(OUTDIR) license
	$(MAKE) OUTDIR=$(OUTDIR) SRVNAME=$(SRVNAME) server
	$(MAKE) TT5DIST=$(TT5DIST) arctar

mac:
	test -f $(TEAMTALK_ROOT)/Client/qtTeamTalk/TeamTalk5.pkg
	mv $(TEAMTALK_ROOT)/Client/qtTeamTalk/TeamTalk5.pkg $(TEAMTALK_ROOT)/Client/qtTeamTalk/TeamTalk_$(VERSION)_Setup.pkg
	$(MAKE) TARGET=$(TEAMTALK_ROOT)/Client/qtTeamTalk/TeamTalk_$(VERSION)_Setup.pkg scp

mac-old:
	$(eval TT5DIST=teamtalk-$(VERSION)-macosx-x86_64)
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	$(eval SRVNAME=$(TEAMTALK_ROOT)/Server/tt5srv)
	$(eval CLIENTNAME=$(TEAMTALK_ROOT)/Client/qtTeamTalk/TeamTalk5.app)
	test -f $(CLIENTNAME)/Contents/Frameworks/libTeamTalk5.dylib
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) license
	make OUTDIR=$(OUTDIR) SRVNAME=$(SRVNAME) server
	make OUTDIR=$(OUTDIR) CLIENTNAME=$(CLIENTNAME) macclient
	make SRCDIR=$(TEAMTALK_ROOT) OUTDIR=$(OUTDIR) maclang
	make arctar

server:
	@echo "########## Copying server ##########"
	mkdir -p $(OUTDIR)/server
	cp $(SRVNAME)                	$(OUTDIR)/server/
	mkdir -p $(OUTDIR)/server/init.d
	cp $(TEAMTALK_ROOT)/Setup/Server/Linux/init.d/tt5server $(OUTDIR)/server/init.d/
	cp $(TEAMTALK_ROOT)/Setup/Server/Linux/init.d/README.md $(OUTDIR)/server/init.d/
	mkdir -p $(OUTDIR)/server/systemd
	cp $(TEAMTALK_ROOT)/Setup/Server/Linux/systemd/tt5server.service $(OUTDIR)/server/systemd/
	cp $(TEAMTALK_ROOT)/Setup/Server/Linux/systemd/README.md $(OUTDIR)/server/systemd/

serverpro:
	@echo "########## Copying serverpro ##########"
	mkdir -p $(OUTDIR)/server
	cp $(SRVNAME)                	$(OUTDIR)/server/
	mkdir -p $(OUTDIR)/server/init.d
	cp $(TEAMTALK_ROOT)/Setup/Server/Linux/init.d/tt5proserver $(OUTDIR)/server/init.d/
	cp $(TEAMTALK_ROOT)/Setup/Server/Linux/init.d/README_pro.md $(OUTDIR)/server/init.d/README.md
	mkdir -p $(OUTDIR)/server/systemd
	cp $(TEAMTALK_ROOT)/Setup/Server/Linux/systemd/tt5proserver.service $(OUTDIR)/server/systemd/
	cp $(TEAMTALK_ROOT)/Setup/Server/Linux/systemd/README_pro.md $(OUTDIR)/server/systemd/README.md
	cp $(TEAMTALK_ROOT)/Setup/Server/Linux/README_professional $(OUTDIR)/README

winserver:
	@echo "########## Copying server ##########"
	mkdir -p $(OUTDIR)/Server
	cp $(SRVNAME)                	$(OUTDIR)/Server/
	cp $(TEAMTALK_ROOT)/Setup/Server/Windows/tt5srv_console.bat $(OUTDIR)/Server/

winserverpro:
	@echo "########## Copying pro server ##########"
	mkdir -p $(OUTDIR)/Server
	cp $(SRVNAME)                	$(OUTDIR)/Server/
	cp $(TEAMTALK_ROOT)/Setup/Server/Windows/tt5prosrv_console.bat $(OUTDIR)/Server/
	cp $(TEAMTALK_ROOT)/Setup/Server/Windows/tt5prosvc_install.bat $(OUTDIR)/Server/
	cp $(TEAMTALK_ROOT)/Setup/Server/Windows/tt5prosvc_uninstall.bat $(OUTDIR)/Server/
	cp $(TEAMTALK_ROOT)/Setup/Server/Windows/README_professional $(OUTDIR)/README.txt

winservice:
	@echo "########## Copying service ##########"
	mkdir -p $(OUTDIR)/Server
	cp $(SVCNAME)                	$(OUTDIR)/Server/
	cp $(TEAMTALK_ROOT)/Setup/Server/Windows/tt5svc_uninstall.bat $(OUTDIR)/Server/
	cp $(TEAMTALK_ROOT)/Setup/Server/Windows/tt5svc_install.bat $(OUTDIR)/Server/

winclient:
	mkdir -p $(OUTDIR)/Client
	cp $(CLIENTEXE) $(OUTDIR)/Client/
	cp $(CLIENTDLL) $(OUTDIR)/Client/
	cp -r $(TEAMTALK_ROOT)/Setup/Client/Sounds $(OUTDIR)/Client/Sounds
	cp $(TEAMTALK_ROOT)/Setup/Client/Windows/TeamTalk5.ini $(OUTDIR)/Client/

debclient:
	mkdir -p $(OUTDIR)/client
	cp $(CLIENTEXE) $(OUTDIR)/client
	cp $(CLIENTDLL) $(OUTDIR)/client
	cp -r $(TEAMTALK_ROOT)/Setup/Client/Sounds $(OUTDIR)/client/sounds
	cp $(TEAMTALK_ROOT)/Setup/Client/Linux/TeamTalk5.ini $(OUTDIR)/client/TeamTalk5.ini
	cp $(TEAMTALK_ROOT)/Setup/Client/Linux/README_debian $(OUTDIR)/client/README
	cp $(TEAMTALK_ROOT)/Setup/Client/Linux/run.sh $(OUTDIR)/client/run.sh
	chmod +x $(OUTDIR)/client/run.sh

deblang:
	lrelease $(SRCDIR)/Client/qtTeamTalk/teamtalk5.pro
	mkdir -p $(OUTDIR)/client/languages
	cp $(SRCDIR)/Client/qtTeamTalk/languages/*.qm $(OUTDIR)/client/languages

winlang:
	test -d $(OUTDIR)/Client/Languages || mkdir -p $(OUTDIR)/Client/Languages
	cp $(SRCDIR)/Client/qtTeamTalk/languages/*.qm $(OUTDIR)/Client/Languages

classichelp:
	cp $(TEAMTALK_ROOT)/Documentation/TeamTalk/output/TeamTalk5.chm $(OUTDIR)/

winhelp:
	mkdir -p $(OUTDIR)/Client
	cp $(TEAMTALK_ROOT)/Documentation/TeamTalk/output/TeamTalk5.chm $(OUTDIR)/Client/

debhelp:
	test -f $(TEAMTALK_ROOT)/Documentation/TeamTalk/output/index.html
	mkdir -p $(OUTDIR)/client/help
	cp $(TEAMTALK_ROOT)/Documentation/TeamTalk/output/* $(OUTDIR)/client/help/

maclang:
	lrelease $(SRCDIR)/Client/qtTeamTalk/teamtalk5.pro
	mkdir -p $(OUTDIR)/client/TeamTalk5.app/Contents/Resources/Languages
	cp $(SRCDIR)/Client/qtTeamTalk/languages/*.qm $(OUTDIR)/client/TeamTalk5.app/Contents/Resources/Languages

license:
	cp $(TEAMTALK_ROOT)/Setup/Installer/Windows/License.txt $(OUTDIR)/License.txt

translator:
	rm -rf tt5_translator
	svn export $(TT5SVN)/Client/qtTeamTalk tt5_translator
	cd tt5_translator && lupdate teamtalk5.pro
	cd tt5_translator && find . -maxdepth 1 -not -name "*.ui" -not -name languages -not -name "*.ts" -exec rm -rf {} \;
	cp $(OUTPUT_DEST)/translators/linguist.exe tt5_translator/
	svn export https://github.com/BearWare/TeamTalk5Setup/trunk/Translate/QtLinguist tt5_translator/manual
	7z a -tzip $(OUTPUT_DEST)/tt5_translator.zip tt5_translator
	rm -rf tt5_translator

	rm -rf tt5classic_translator
	mkdir tt5classic_translator
	cp $(OUTPUT_DEST)/translators/LanguageEditor.exe tt5classic_translator/
	svn export https://github.com/BearWare/TeamTalk5Setup/trunk/Translate/LanguageEditor tt5classic_translator/manual
	7z a -tzip $(OUTPUT_DEST)/tt5classic_translator.zip tt5classic_translator
	rm -rf tt5classic_translator
