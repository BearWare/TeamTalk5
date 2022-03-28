VERSION="v5.9.0.5079"

OUTPUT_DEST=./
INSTALLDIR?=$(TEAMTALK_INSTALLDIR)

first:
	echo "No target"

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

win32:
	$(eval TT5DIST=TeamTalk_$(VERSION)_Portable)
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) license winserver winclient
	cp $(INSTALLDIR)/Client/qtTeamTalk/dolapi32.dll $(OUTDIR)/Client/
	cp $(INSTALLDIR)/Client/qtTeamTalk/nvdaControllerClient32.dll $(OUTDIR)/Client/
	cp $(INSTALLDIR)/Client/qtTeamTalk/SAAPI32.dll $(OUTDIR)/Client/
	cp $(INSTALLDIR)/Client/qtTeamTalk/Tolk.dll $(OUTDIR)/Client/
	make TT5DIST=$(TT5DIST) arczip

win64:
	$(eval TT5DIST=TeamTalk_$(VERSION)_Portable)
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) license winserver winclient
	cp $(INSTALLDIR)/Client/qtTeamTalk/nvdaControllerClient64.dll $(OUTDIR)/Client/
	cp $(INSTALLDIR)/Client/qtTeamTalk/SAAPI64.dll $(OUTDIR)/Client/
	cp $(INSTALLDIR)/Client/qtTeamTalk/Tolk.dll $(OUTDIR)/Client/
	make TT5DIST=$(TT5DIST) arczip

win32pro:
	$(eval TT5DIST=teamtalkpro-$(VERSION)-win32)
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) winserverpro
	make TT5DIST=$(TT5DIST) arczip

win64pro:
	$(eval TT5DIST=teamtalkpro-$(VERSION)-win64)
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) winserverpro
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
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) license linuxserver linuxclient
	make arctar

linuxpro:
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) linuxserverpro
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
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	make OUTDIR=$(OUTDIR) license linuxserver
	make arctar

centos7:
	$(eval TT5DIST=teamtalk-$(VERSION)-centos7-x86_64)
	$(eval OUTDIR=$(OUTPUT_DEST)/$(TT5DIST))
	rm -rf $(OUTDIR)
	mkdir -p $(OUTDIR)
	$(MAKE) OUTDIR=$(OUTDIR) license linuxserver
	$(MAKE) TT5DIST=$(TT5DIST) arctar

linuxserver:
	@echo "########## Copying server ##########"
	mkdir -p $(OUTDIR)/server
	cp $(INSTALLDIR)/Server/tt5srv $(OUTDIR)/server/
	strip $(OUTDIR)/server/tt5srv
	mkdir -p $(OUTDIR)/server/init.d
	cp $(TEAMTALK_ROOT)/Setup/Server/Linux/init.d/tt5server $(OUTDIR)/server/init.d/
	cp $(TEAMTALK_ROOT)/Setup/Server/Linux/init.d/README.md $(OUTDIR)/server/init.d/
	mkdir -p $(OUTDIR)/server/systemd
	cp $(TEAMTALK_ROOT)/Setup/Server/Linux/systemd/tt5server.service $(OUTDIR)/server/systemd/
	cp $(TEAMTALK_ROOT)/Setup/Server/Linux/systemd/README.md $(OUTDIR)/server/systemd/

linuxserverpro:
	@echo "########## Copying serverpro ##########"
	mkdir -p $(OUTDIR)/server
	cp $(INSTALLDIR)/Server/tt5prosrv $(OUTDIR)/server/
	strip $(OUTDIR)/server/tt5prosrv
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
	cp $(INSTALLDIR)/Server/tt5srv.exe $(OUTDIR)/Server/
	cp $(TEAMTALK_ROOT)/Setup/Server/Windows/tt5srv_console.bat $(OUTDIR)/Server/
	cp $(INSTALLDIR)/Server/tt5svc.exe $(OUTDIR)/Server/
	cp $(TEAMTALK_ROOT)/Setup/Server/Windows/tt5svc_uninstall.bat $(OUTDIR)/Server/
	cp $(TEAMTALK_ROOT)/Setup/Server/Windows/tt5svc_install.bat $(OUTDIR)/Server/

winserverpro:
	@echo "########## Copying pro server ##########"
	mkdir -p $(OUTDIR)/Server
	cp $(INSTALLDIR)/Server/tt5prosrv.exe $(OUTDIR)/Server/
	cp $(TEAMTALK_ROOT)/Setup/Server/Windows/tt5prosrv_console.bat $(OUTDIR)/Server/
	cp $(INSTALLDIR)/Server/tt5prosvc.exe $(OUTDIR)/Server/
	cp $(TEAMTALK_ROOT)/Setup/Server/Windows/tt5prosvc_install.bat $(OUTDIR)/Server/
	cp $(TEAMTALK_ROOT)/Setup/Server/Windows/tt5prosvc_uninstall.bat $(OUTDIR)/Server/
	cp $(TEAMTALK_ROOT)/Setup/Server/Windows/README_professional $(OUTDIR)/README.txt

winclient:
	mkdir -p $(OUTDIR)/Client
# Qt client
	cp $(INSTALLDIR)/Client/qtTeamTalk/TeamTalk5.exe $(OUTDIR)/Client/
	cp -r $(INSTALLDIR)/Client/qtTeamTalk/languages $(OUTDIR)/Client/
	cp $(TEAMTALK_ROOT)/Setup/Client/Windows/TeamTalk5.ini $(OUTDIR)/Client/
	cp $(TEAMTALK_ROOT)/Setup/Client/Windows/TeamTalk5.ini $(OUTDIR)/Client/TeamTalk5.ini.default
# Classic client
	cp $(INSTALLDIR)/Client/TeamTalkClassic/TeamTalk5Classic.exe $(OUTDIR)/Client/
	cp $(TEAMTALK_ROOT)/Setup/Client/Windows/TeamTalk5Classic.xml $(OUTDIR)/Client/
	cp $(TEAMTALK_ROOT)/Setup/Client/Windows/TeamTalk5Classic.xml $(OUTDIR)/Client/TeamTalk5Classic.xml.default
	cp -r $(INSTALLDIR)/Client/TeamTalkClassic/lng_classic $(OUTDIR)/Client/
# Shared files
	cp $(INSTALLDIR)/Library/TeamTalk_DLL/TeamTalk5.dll $(OUTDIR)/Client/
	cp -r $(TEAMTALK_ROOT)/Setup/Client/Sounds $(OUTDIR)/Client/Sounds
	cp $(TEAMTALK_ROOT)/Documentation/TeamTalk/output/TeamTalk5.chm $(OUTDIR)/Client/

linuxclient:
	mkdir -p $(OUTDIR)/client
	cp $(INSTALLDIR)/Client/qtTeamTalk/teamtalk5 $(OUTDIR)/client
	strip $(OUTDIR)/client/teamtalk5
	cp $(TEAMTALK_ROOT)/Setup/Client/Linux/TeamTalk5.ini $(OUTDIR)/client/
	cp $(TEAMTALK_ROOT)/Setup/Client/Linux/TeamTalk5.ini $(OUTDIR)/client/TeamTalk5.ini.default
	cp $(INSTALLDIR)/Library/TeamTalk_DLL/libTeamTalk5.so $(OUTDIR)/client
	strip $(OUTDIR)/client/libTeamTalk5.so
	mkdir -p $(OUTDIR)/client/languages
	cp -r $(INSTALLDIR)/Client/qtTeamTalk/languages $(OUTDIR)/client/
	cp -r $(TEAMTALK_ROOT)/Setup/Client/Sounds $(OUTDIR)/client/sounds
	cp $(TEAMTALK_ROOT)/Setup/Client/Linux/README_debian $(OUTDIR)/client/README
	cp $(TEAMTALK_ROOT)/Setup/Client/Linux/run.sh $(OUTDIR)/client/run.sh
	chmod +x $(OUTDIR)/client/run.sh
	mkdir -p $(OUTDIR)/client/help
	cp -r $(TEAMTALK_ROOT)/Documentation/TeamTalk/output/* $(OUTDIR)/client/help/

license:
	cp $(TEAMTALK_ROOT)/Setup/Installer/Windows/License.txt $(OUTDIR)/License.txt
