/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Net;
using System.IO;
using System.Windows.Forms;
using System.Xml;
using System.Reflection;
using BearWare;

namespace TeamTalkApp.NET
{
    public partial class ConnectDlg : Form
    {
        TeamTalk ttclient;
        Settings settings;

        List<Server> publicservers = new List<Server>();

        public ConnectDlg(TeamTalk tt, Settings settings)
        {
            ttclient = tt;
            this.settings = settings;
            InitializeComponent();

            ipaddrComboBox.Text = settings.server.ipaddr;
            tcpportNumericUpDown.Value = settings.server.tcpport;
            udpportNumericUpDown.Value = settings.server.udpport;
            cryptCheckBox.Checked = settings.server.encrypted;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            settings.server.ipaddr = ipaddrComboBox.Text;
            settings.server.tcpport = (int)tcpportNumericUpDown.Value;
            settings.server.udpport = (int)udpportNumericUpDown.Value;
            settings.server.encrypted = cryptCheckBox.Checked;

            /* 
             * In TeamTalk 5 Standard Edition it's only possible to connect
             * to non-encrypted servers.
             */
            if (!ttclient.ConnectEx(ipaddrComboBox.Text, (int)tcpportNumericUpDown.Value,
                            (int)udpportNumericUpDown.Value, "", 0, 0, settings.server.encrypted))
                MessageBox.Show("Failed to connect");
        }

        private void ConnectDlg_Load(object sender, EventArgs e)
        {
            this.CenterToScreen();

            getServers();
        }

        private void getServers()
        {
            //get list of public server for test purposes
            Assembly exeasm = Assembly.GetExecutingAssembly();
            AssemblyName exeasmname = exeasm.GetName();
            string exename = exeasmname.Name;
            string exeversion = exeasmname.Version.ToString();

            Assembly dllasm = Assembly.GetAssembly(ttclient.GetType());
            AssemblyName dllasmname = dllasm.GetName();
            string dllname = dllasmname.Name;
            string dllversion = dllasmname.Version.ToString();
            string url = string.Format("http://www.bearware.dk/teamtalk/tt5servers.php?client={0}&version={1}&dllversion={2}&os=Windows",
                                       dllname, exeversion, dllversion);

            HttpWebRequest myHttpWebRequest1 = (HttpWebRequest)WebRequest.Create(url);
            WebResponse response = myHttpWebRequest1.GetResponse();
            Stream dataStream = response.GetResponseStream();
            StreamReader reader = new StreamReader(dataStream);
            string responseFromServer = reader.ReadToEnd();
            parsePublicServers(responseFromServer);

            foreach(Server server in publicservers)
            {
                serversListBox.Items.Add(server);
            }
        }

        private void parsePublicServers(string xml)
        {
            XmlDocument xmldoc = new XmlDocument();
            xmldoc.LoadXml(xml);

            XmlNodeList hosts = xmldoc.GetElementsByTagName("host");
            foreach (XmlNode host in hosts)
            {
                Server server = new Server();
                foreach (XmlNode node in host.ChildNodes)
                {
                    switch (node.Name)
                    {
                        case "name":
                            server.name = node.InnerText;
                            break;
                        case "address":
                            server.ipaddr = node.InnerText;
                            break;
                        case "tcpport":
                            server.tcpport = int.Parse(node.InnerText);
                            break;
                        case "udpport":
                            server.udpport = int.Parse(node.InnerText);
                            break;
                        case "encrypted" :
                            server.encrypted = node.InnerText == "true";
                            break;
                        case "auth":
                            foreach (XmlNode auth in node.ChildNodes)
                            {
                                switch (auth.Name)
                                {
                                    case "username":
                                        server.username = auth.InnerText;
                                        break;
                                    case "password":
                                        server.password = auth.InnerText;
                                        break;
                                }
                            }
                            break;
                        case "join":
                            foreach (XmlNode join in node.ChildNodes)
                            {
                                switch (join.Name)
                                {
                                    case "channel":
                                        server.channel = join.InnerText;
                                        break;
                                    case "password":
                                        server.chanpasswd = join.InnerText;
                                        break;
                                }
                            }
                            break;
                    }
                }
                publicservers.Add(server);
            }
        }

        private void serversListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            settings.server = publicservers[serversListBox.SelectedIndex];
            ipaddrComboBox.Text = settings.server.ipaddr;
            tcpportNumericUpDown.Value = settings.server.tcpport;
            udpportNumericUpDown.Value = settings.server.udpport;
            cryptCheckBox.Checked = settings.server.encrypted;
        }
    }
}
