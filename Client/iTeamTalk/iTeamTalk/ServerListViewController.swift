/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

import SwiftUI
import UIKit

enum ServerType {
    case LOCAL,
    OFFICIAL,
    PUBLIC,
    UNOFFICIAL
}

// Properties of a TeamTalk server to connect to
class Server : NSObject, NSSecureCoding {
    static var supportsSecureCoding: Bool {
        true
    }
    
    var name = ""
    var ipaddr = ""
    var tcpport = AppInfo.DEFAULT_TCPPORT
    var udpport = AppInfo.DEFAULT_UDPPORT
    var joincode = ""
    var nickname = ""
    var username = ""
    var password = ""
    var channel = ""
    var chanpasswd = ""
    var servertype : ServerType = .LOCAL
    var encrypted = false
    var cacertdata = ""
    var certdata = ""
    var certprivkeydata = ""
    var certverifypeer = false
    
    // stats from public server list
    var stats_usercount = 0
    var stats_country = ""
    var stats_servername = ""
    var stats_motd = ""
    
    override init() {
        
    }
    
    required init?(coder dec: NSCoder) {
        name = Server.decodeString(dec, forKey: "name")
        ipaddr = Server.decodeString(dec, forKey: "ipaddr")
        tcpport = dec.decodeInteger(forKey: "tcpport")
        udpport = dec.decodeInteger(forKey: "udpport")
        username = Server.decodeString(dec, forKey: "username")
        password = Server.decodeString(dec, forKey: "password")
        nickname = Server.decodeString(dec, forKey: "nickname")
        channel = Server.decodeString(dec, forKey: "channel")
        chanpasswd = Server.decodeString(dec, forKey: "chanpasswd")
        encrypted = dec.decodeBool(forKey: "encrypted")
        cacertdata = Server.decodeString(dec, forKey: "cacertdata")
        certdata = Server.decodeString(dec, forKey: "certdata")
        certprivkeydata = Server.decodeString(dec, forKey: "certprivkeydata")
        certverifypeer = dec.decodeBool(forKey: "certverifypeer")
    }
    
    func encode(with enc: NSCoder) {
        enc.encode(name, forKey: "name")
        enc.encode(ipaddr, forKey: "ipaddr")
        enc.encode(tcpport, forKey: "tcpport")
        enc.encode(udpport, forKey: "udpport")
        enc.encode(username, forKey: "username")
        enc.encode(password, forKey: "password")
        enc.encode(nickname, forKey: "nickname")
        enc.encode(channel, forKey: "channel")
        enc.encode(chanpasswd, forKey: "chanpasswd")
        enc.encode(encrypted, forKey: "encrypted")
        enc.encode(cacertdata, forKey: "cacertdata")
        enc.encode(certdata, forKey: "certdata")
        enc.encode(certprivkeydata, forKey: "certprivkeydata")
        enc.encode(certverifypeer, forKey: "certverifypeer")
    }
    
    private static func decodeString(_ decoder: NSCoder, forKey key: String) -> String {
        if let value = decoder.decodeObject(of: NSString.self, forKey: key) {
            return value as String
        }
        return ""
    }
}

func loadLocalServers() -> [Server] {
    
    var servers = [Server]()
    
    let defaults = UserDefaults.standard
    if let stored = defaults.array(forKey: "ServerList") {
        for e in stored {
            let data = e as! Data
            
            if let server = try? NSKeyedUnarchiver.unarchivedObject(ofClass: Server.self, from: data) {
                servers.append(server)
            }
        }
    }
    return servers
}

func saveLocalServers(_ servers : [Server]) {
    //Store local servers
    let defaults = UserDefaults.standard
    var s_array = [Data]()
    for s in servers {
        if let data = try? NSKeyedArchiver.archivedData(withRootObject: s, requiringSecureCoding: true) {
            s_array.append(data)
        }
    }
    defaults.set(s_array, forKey: "ServerList")
    defaults.synchronize()
}

final class ServerListViewController: UIHostingController<ServerListView> {

    var currentServer = Server()
    // list of available servers
    var servers = [Server]() {
        didSet { model.servers = servers }
    }

    var nextappupdate = Date()
    private var model: ServerListModel

    init() {
        let model = ServerListModel(servers: [])
        self.model = model
        super.init(rootView: ServerListView(
            model: model,
            enterJoinCode: { },
            showServer: { _ in },
            connectServer: { _ in },
            deleteServer: { _ in }
        ))
    }

    required init?(coder: NSCoder) { fatalError("use init()") }

    override func viewDidLoad() {
        super.viewDidLoad()

        rootView = ServerListView(
            model: model,
            enterJoinCode: { [weak self] in self?.enterJoinCode() },
            showServer: { [weak self] server in self?.showServer(server) },
            connectServer: { [weak self] server in self?.initiateConnect(to: server) },
            deleteServer: { [weak self] server in self?.deleteServer(server) }
        )

        navigationItem.title = NSLocalizedString("TeamTalk Servers", comment: "serverlist")

        let setupBtn = UIBarButtonItem(
            image: UIImage(named: "setup"),
            style: .plain,
            target: self,
            action: #selector(openPreferences)
        )
        setupBtn.accessibilityLabel = NSLocalizedString("Preferences", comment: "serverlist")
        setupBtn.accessibilityHint = NSLocalizedString("Access preferences", comment: "serverlist")
        navigationItem.leftBarButtonItem = setupBtn

        let addBtn = UIBarButtonItem(
            barButtonSystemItem: .add,
            target: self,
            action: #selector(addServer)
        )
        addBtn.accessibilityLabel = NSLocalizedString("Add new server entry", comment: "serverlist")
        navigationItem.rightBarButtonItem = addBtn

        NotificationCenter.default.addObserver(self, selector: #selector(handleOpenURL(_:)), name: .iTeamTalkOpenURL, object: nil)
    }

    @objc private func openPreferences() {
        let model = PreferencesModel()
        let vc = UIHostingController(rootView: PreferencesView(model: model))
        navigationController?.pushViewController(vc, animated: true)
    }

    @objc private func addServer() {
        pushServerDetail(for: Server())
    }

    private func showServer(_ server: Server) {
        pushServerDetail(for: server)
    }

    private func pushServerDetail(for server: Server) {
        let vc = ServerDetailViewController()
        vc.server = server
        vc.onConnect = { [weak self] updatedServer in
            guard let self else { return }
            upsertServer(updatedServer)
            currentServer = updatedServer
            showChannelList()
        }
        vc.onDelete = { [weak self] in
            guard let self else { return }
            servers.removeAll { $0 === server }
            saveServerList()
            navigationController?.popViewController(animated: true)
        }
        navigationController?.pushViewController(vc, animated: true)
    }

    private func upsertServer(_ server: Server) {
        if let idx = servers.firstIndex(where: { $0.name == server.name }) {
            servers[idx] = server
        } else {
            servers.append(server)
        }
        saveServerList()
    }

    private func initiateConnect(to server: Server) {
        currentServer = server
        showChannelList()
    }

    private func showChannelList() {
        let vc = MainTabBarController(server: currentServer)
        navigationController?.pushViewController(vc, animated: true)
    }

    @objc private func handleOpenURL(_ notification: Notification) {
        guard let url = notification.object as? URL else { return }
        navigationController?.popToRootViewController(animated: true)
        openUrl(url)
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)

        servers.removeAll()
        
        servers = loadLocalServers()
        
        let defaults = UserDefaults.standard
        var downloadServers = defaults.object(forKey: PREF_DISPLAY_OFFICIALSERVERS) == nil || defaults.bool(forKey: PREF_DISPLAY_OFFICIALSERVERS)
        downloadServers = downloadServers || (defaults.object(forKey: PREF_DISPLAY_PUBLICSERVERS) == nil || defaults.bool(forKey: PREF_DISPLAY_PUBLICSERVERS))
        downloadServers = downloadServers || (defaults.object(forKey: PREF_DISPLAY_UNOFFICIALSERVERS) != nil && defaults.bool(forKey: PREF_DISPLAY_UNOFFICIALSERVERS))
        if downloadServers {
            Timer.scheduledTimer(timeInterval: 1.0, target: self, selector: #selector(ServerListViewController.downloadServerList), userInfo: nil, repeats: false)
        }

        model.servers = servers
        
        if (nextappupdate as NSDate).earlierDate(Date()) == nextappupdate {
            Timer.scheduledTimer(timeInterval: 2.0, target: self, selector: #selector(ServerListViewController.checkAppUpdate), userInfo: nil, repeats: false)
        }
        
    }
    
    @objc func checkAppUpdate() {
        
        // check for new version
        if let url = URL(string: AppInfo.getUpdateURL()) {
            let task = URLSession.shared.dataTask(with: url) { data, response, error in
                if error != nil {
                    print ("Failed to check app update \(url.absoluteString)")
                    return
                }
                guard let httpResponse = response as? HTTPURLResponse,
                      (200...299).contains(httpResponse.statusCode) else {
                    print ("Failed to check app update \(url.absoluteString)")
                    return
                }
                if let data = data {
                    let parser = XMLParser(data: data)
                    let updateparser = AppUpdateParser()
                    parser.delegate = updateparser
                    parser.parse()
                    if updateparser.registerUrl.isEmpty == false {
                        AppInfo.BEARWARE_REGISTRATION_WEBSITE = updateparser.registerUrl
                    }
                }
            }
            task.resume()
        }
        nextappupdate = nextappupdate.addingTimeInterval(60 * 60 * 24)
    }
    
    @objc func downloadServerList() {

        // get xml-list of public server
       
        let defaults = UserDefaults.standard
        let official = defaults.object(forKey: PREF_DISPLAY_OFFICIALSERVERS) == nil || defaults.bool(forKey: PREF_DISPLAY_OFFICIALSERVERS)
        let unofficial = defaults.object(forKey: PREF_DISPLAY_UNOFFICIALSERVERS) != nil && defaults.bool(forKey: PREF_DISPLAY_UNOFFICIALSERVERS)

        if let serversurl = URL(string: AppInfo.getServersURL(officialservers: official, unofficialservers: unofficial)) {
        
            let task = URLSession.shared.dataTask(with: serversurl) { data, response, error in
                if error != nil {
                    print ("Failed to download server list \(serversurl.absoluteString)")
                    return
                }
                guard let httpResponse = response as? HTTPURLResponse,
                      (200...299).contains(httpResponse.statusCode) else {
                    print ("Failed to download server list \(serversurl.absoluteString)")
                    return
                }
                if let data = data {
                    DispatchQueue.main.async {
                        self.insertServerList(data: data)
                    }
                }
            }
            task.resume()
        }
    }
    
    func insertServerList(data: Data) {
        let parser = XMLParser(data: data)
        let serverparser = ServerParser()
        parser.delegate = serverparser
        parser.parse()
        
        for s in serverparser.servers {
            servers.append(s)
        }
        model.servers = servers
    }
    
    func downloadServer(joincode: String) {
        if let joincodeurl = URL(string: AppInfo.getBearWareJoinCode(joincode: joincode)) {
            let task = URLSession.shared.dataTask(with: joincodeurl) { data, response, error in
                if error != nil {
                    print ("Failed to download server list \(joincodeurl.absoluteString)")
                    return
                }
                guard let httpResponse = response as? HTTPURLResponse,
                      (200...299).contains(httpResponse.statusCode) else {
                    print ("Failed to download server list \(joincodeurl.absoluteString)")
                    return
                }
                if let data = data {
                    DispatchQueue.main.async {
                        let parser = XMLParser(data: data)
                        let serverparser = ServerParser()
                        parser.delegate = serverparser
                        parser.parse()
                        
                        if serverparser.currentServer.ipaddr.isEmpty == false {
                            self.currentServer = serverparser.currentServer
                            self.showChannelList()
                        } else {
                            let alert = UIAlertController(title: NSLocalizedString("Connect to Server", comment: "serverlist"),
                                                          message: NSLocalizedString("No server found", comment: "serverlist"),
                                                          preferredStyle: .alert)
                            let okAction = UIAlertAction(title: "OK", style: .default) { _ in }
                            alert.addAction(okAction)
                            self.present(alert, animated: true)
                        }
                    }
                }
            }
            task.resume()
        }
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    func saveServerList() {
        
        saveLocalServers(servers.filter({$0.servertype == .LOCAL}))
    }
    
    @objc
    func connectServer(_ action: UIAccessibilityCustomAction) -> Bool {
        if let ac = action as? MyCustomAction {
            initiateConnect(to: servers[ac.tag])
        }
        return true
    }
    
    @objc
    func deleteServer(_ action: UIAccessibilityCustomAction) -> Bool {
        
        if let ac = action as? MyCustomAction {
            deleteServer(servers[ac.tag])
        }
        return true
    }

    private func deleteServer(_ server: Server) {
        servers.removeAll { $0 === server }
        saveServerList()
    }

    func enterJoinCode() {
        let alert = UIAlertController(title: NSLocalizedString("Connect to Server", comment: "serverlist"),
                                      message: NSLocalizedString("Enter Join Code", comment: "serverlist"),
                                      preferredStyle: .alert)
        alert.addTextField {
            textField in
            textField.placeholder = NSLocalizedString("Type Join Code", comment: "serverlist")
            textField.autocorrectionType = .no
            if let clipboardText = UIPasteboard.general.string {
                textField.text = clipboardText
            }
        }
        
        // Add an "OK" action
        let okAction = UIAlertAction(title: "OK", style: .default) { _ in
            if let text = alert.textFields?.first?.text {
                self.downloadServer(joincode: text)
            }
        }

        // Add a "Cancel" action
        let cancelAction = UIAlertAction(title: "Cancel", style: .cancel) { _ in }

        alert.addAction(okAction)
        alert.addAction(cancelAction)
        self.present(alert, animated: true)
    }
    
    func openUrl(_ url: URL) {
        
        if url.isFileURL {
            // get server from either .tt file or tt-URL
            let serverparser = ServerParser()
            
            if let parser = XMLParser(contentsOf: url) {
                parser.delegate = serverparser
                parser.parse()
            }
            
            for s in serverparser.servers {
                currentServer = s
            }
        }
        else if url.absoluteString.starts(with: AppInfo.TTLINK_PREFIX){
            do {
                // assume TT url
                let url_str = url.absoluteString
                let ns_str = url_str as NSString
                let url_range = NSMakeRange(0, url_str.count)

                // ip-addr
                let host = AppInfo.TTLINK_PREFIX + "([^\\??!/]*)/?\\??"
                let host_regex = try NSRegularExpression(pattern: host, options: .caseInsensitive)
                let host_matches = host_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = host_matches.first {
                    currentServer.ipaddr = ns_str.substring(with: m.range(at: 1))
                }
                
                //tcp port
                let tcpport = "[&|\\?]tcpport=(\\d+)"
                let tcpport_regex = try NSRegularExpression(pattern: tcpport, options: .caseInsensitive)
                let tcpport_matches = tcpport_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = tcpport_matches.first {
                    let s = ns_str.substring(with: m.range(at: 1))
                    currentServer.tcpport = Int(s)!
                }
                
                // udp port
                let udpport = "[&|\\?]udpport=(\\d+)"
                let udpport_regex = try NSRegularExpression(pattern: udpport, options: .caseInsensitive)
                let udpport_matches = udpport_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = udpport_matches.first {
                    let s = ns_str.substring(with: m.range(at: 1))
                    currentServer.udpport = Int(s)!
                }

                // encrypted
                let encrypted = "[&|\\?]encrypted=([^&]*)"
                let encrypted_regex = try NSRegularExpression(pattern: encrypted, options: .caseInsensitive)
                let encrypted_matches = encrypted_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = encrypted_matches.first {
                    let str = ns_str.substring(with: m.range(at: 1))
                    currentServer.encrypted = str == "true" || str == "1"
                }

                // username
                let username = "[&|\\?]username=([^&]*)"
                let username_regex = try NSRegularExpression(pattern: username, options: .caseInsensitive)
                let username_matches = username_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = username_matches.first {
                    let urlusername = ns_str.substring(with: m.range(at: 1))
                    currentServer.username = urlusername.removingPercentEncoding!
                }
                
                // password
                let password = "[&|\\?]password=([^&]*)"
                let password_regex = try NSRegularExpression(pattern: password, options: .caseInsensitive)
                let password_matches = password_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = password_matches.first {
                    let urlpasswd = ns_str.substring(with: m.range(at: 1))
                    currentServer.password = urlpasswd.removingPercentEncoding!
                }

                // nickname
                let nickname = "[&|\\?]nickname=([^&]*)"
                let nickname_regex = try NSRegularExpression(pattern: nickname, options: .caseInsensitive)
                let nickname_matches = nickname_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = nickname_matches.first {
                    let urlnickname = ns_str.substring(with: m.range(at: 1))
                    currentServer.nickname = urlnickname.removingPercentEncoding!
                }

                // channel
                let channel = "[&|\\?]channel=([^&]*)"
                let channel_regex = try NSRegularExpression(pattern: channel, options: .caseInsensitive)
                let channel_matches = channel_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = channel_matches.first {
                    let urlchannel = ns_str.substring(with: m.range(at: 1))
                    currentServer.channel = urlchannel.removingPercentEncoding!
                }
                
                // channel password
                let chpasswd = "[&|\\?]chanpasswd=([^&]*)"
                let chpasswd_regex = try NSRegularExpression(pattern: chpasswd, options: .caseInsensitive)
                let chpasswd_matches = chpasswd_regex.matches(in: url_str, options: .reportCompletion, range: url_range)
                if let m = chpasswd_matches.first {
                    let urlchpasswd = ns_str.substring(with: m.range(at: 1))
                    currentServer.chanpasswd = urlchpasswd.removingPercentEncoding!
                }
            }
            catch {
                
            }
        }
        
        if !currentServer.ipaddr.isEmpty {
            showChannelList()
        }
    }
}

final class ServerListModel: ObservableObject {
    @Published var servers: [Server]

    init(servers: [Server]) {
        self.servers = servers
    }
}

struct ServerListView: View {
    @ObservedObject var model: ServerListModel

    let enterJoinCode: () -> Void
    let showServer: (Server) -> Void
    let connectServer: (Server) -> Void
    let deleteServer: (Server) -> Void

    var body: some View {
        List {
            TeamTalkActionRow(title: NSLocalizedString("Enter Join Code", comment: "serverlist"), action: enterJoinCode)

            ForEach(model.servers, id: \.self) { server in
                TeamTalkServerRow(
                    title: server.name,
                    subtitle: detail(for: server),
                    iconName: iconName(for: server),
                    iconAccessibilityLabel: iconAccessibilityLabel(for: server),
                    actionTitle: NSLocalizedString("Connect", comment: "serverlist")
                ) {
                    connectServer(server)
                }
                .contentShape(Rectangle())
                .onTapGesture {
                    showServer(server)
                }
                .swipeActions(edge: .trailing) {
                    Button(role: .destructive) {
                        deleteServer(server)
                    } label: {
                        Label(NSLocalizedString("Delete", comment: "serverlist"), systemImage: "trash")
                    }
                }
                .accessibilityAction(named: NSLocalizedString("Connect to server", comment: "serverlist")) {
                    connectServer(server)
                }
                .accessibilityAction(named: NSLocalizedString("Delete server from list", comment: "serverlist")) {
                    deleteServer(server)
                }
            }
        }
    }

    private func detail(for server: Server) -> String {
        var detail = "\(server.ipaddr):\(server.tcpport)"
        if server.servertype != .LOCAL {
            detail += ", " + String(format: NSLocalizedString("Users: %d, Country: %@", comment: "serverlist"), server.stats_usercount, server.stats_country)
        }
        return detail
    }

    private func iconName(for server: Server) -> String {
        switch server.servertype {
        case .LOCAL:
            return "teamtalk_yellow.png"
        case .OFFICIAL:
            return "teamtalk_blue.png"
        case .PUBLIC:
            return "teamtalk_green.png"
        case .UNOFFICIAL:
            return "teamtalk_orange.png"
        }
    }

    private func iconAccessibilityLabel(for server: Server) -> String {
        switch server.servertype {
        case .LOCAL:
            return NSLocalizedString("Local server", comment: "serverlist")
        case .OFFICIAL:
            return NSLocalizedString("Official server", comment: "serverlist")
        case .PUBLIC:
            return NSLocalizedString("Public server", comment: "serverlist")
        case .UNOFFICIAL:
            return NSLocalizedString("Unofficial server", comment: "serverlist")
        }
    }
}

class AppUpdateParser : NSObject, XMLParserDelegate {

    var elementStack = [String]()
    var update = ""
    var registerUrl = ""
    
    func parser(_ parser: XMLParser, didStartElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?,
        attributes attributeDict: [String : String]) {
        
        elementStack.append(elementName)
    }

    func parser(_ parser: XMLParser, foundCharacters string: String) {
        let path = getXMLPath(elementStack: elementStack)
        switch path {
        case "/teamtalk/update/name" :
            update = string
        case "/teamtalk/bearware/register-url" :
            registerUrl = string
        default :
             print("Unknown path " + path)
        }
    }

    func parser(_ parser: XMLParser, didEndElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?) {
        
        self.elementStack.removeLast()
    }

}

class ServerParser : NSObject, XMLParserDelegate {
    
    var currentServer = Server()
    var elementStack = [String]()
    var servers = [Server]()
    
    func parser(_ parser: XMLParser, didStartElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?,
        attributes attributeDict: [String : String]) {
        
        elementStack.append(elementName)
        if elementName == "host" {
            currentServer = Server()
        }
    }
    
    func parser(_ parser: XMLParser, foundCharacters string: String) {
        
        switch elementStack.last! {
        case "teamtalk" : break
        case "host" : break
        // <host>
        case "id" : break
        case "name" :
            currentServer.name = string
        case "address" :
            currentServer.ipaddr = string
        case "tcpport" :
            let v : String = string
            currentServer.tcpport = Int(v)!
        case "udpport" :
            let v : String = string
            currentServer.udpport = Int(v)!
        case "encrypted" :
            currentServer.encrypted = string == "true"
        case "joincode" :
            currentServer.joincode = string
        case "listing" :
            if string == "official" {
                currentServer.servertype = .OFFICIAL
            }
            else if string == "public" {
                currentServer.servertype = .PUBLIC
            }
            else if string == "private" {
                currentServer.servertype = .UNOFFICIAL
            }
        case "auth" : break
        case "join" : break
        // <auth>
        case "username" :
            currentServer.username = string
        // <auth> or <join>
        case "password" :
            if elementStack.firstIndex(of: "auth") != nil {
                currentServer.password = string
            }
            else if elementStack.firstIndex(of: "join") != nil {
                currentServer.chanpasswd = string
            }
        // <auth>
        case "nickname" :
            currentServer.nickname = string
        case "channel" :
            currentServer.channel = string

        // <stats>
        case "stats" : break
        case "user-count" :
            currentServer.stats_usercount = Int(string) ?? 0
        case "country" :
            currentServer.stats_country = string
        case "servername" :
            currentServer.stats_servername += string
        case "motd" :
            currentServer.stats_motd += string
        case "active" :
            break
            
        // <trusted-certificate>
        case "trusted-certificate" : break
        case "certificate-authority-pem" :
            currentServer.cacertdata += string
        case "client-certificate-pem" :
            currentServer.certdata += string
        case "client-private-key-pem" :
            currentServer.certprivkeydata += string
        case "verify-peer" :
            currentServer.certverifypeer = string == "true"
        default :
            print("Unknown tag " + self.elementStack.last!)
        }
    }
    
    func parser(_ parser: XMLParser, didEndElement elementName: String,
        namespaceURI: String?, qualifiedName qName: String?) {
            
            self.elementStack.removeLast()
            if elementName == "host" {
                servers.append(currentServer)
            }
    }
    
}
