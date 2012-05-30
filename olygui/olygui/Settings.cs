using System;
using System.IO;

namespace olygui {

    public class Settings {

        // Game Settings
        private static string gameSettingsFile = Directory.GetCurrentDirectory() + "\\game.ini";
        private static string gameName = "";
        private static bool gameJoinByEmail = false;
        private static string gameJoinByEmailPw = "changethis";
        private static bool gameJoinByEmailReplyWhenError = false;
        private static string gameHtmlReportsFolder = Directory.GetCurrentDirectory() + "\\lib\\html";
        private static string gamePublicHtmlReportsFolder = Directory.GetCurrentDirectory() + "\\public";
        private static string gameHtmlTimesFolder = Directory.GetCurrentDirectory() + "\\times";
        private static bool gameRunTurnByEmail = true;
        private static string gameRunTurnByEmailPw = "changethis";

        // Incoming E-mail
        private static string incMailSettingsFile = Directory.GetCurrentDirectory() + "\\getmail.ini";
        private static string incMailServer = "pop.gmail.com";
        private static bool incMailSSL = true;
        private static uint incMailPort = 995;
        private static string incMailUser = "";
        private static string incMailPw = "";
        private static bool incMailLeaveCopyOnServer = false;

        // Outgoing E-mail
        private static string outMailSettingsFile = Directory.GetCurrentDirectory() + "\\sendmail.ini";
        private static string outMailHost = "localhost";
        private static bool outMailSSL = false;
        private static uint outMailPort = 25;
        private static string outMailUser = "";
        private static string outMailPw = "";
        private static bool outMailSendMail = true;

        public static string GameName { get { return gameName; } set { gameName = value; } }
        public static bool GameJoinByEmail { get { return gameJoinByEmail; } set { gameJoinByEmail = value; } }
        public static string GameJoinByEmailPw { get { return gameJoinByEmailPw; } set { gameJoinByEmailPw = value; } }
        public static bool GameJoinByEmailReplyWhenError { get { return gameJoinByEmailReplyWhenError; } set { gameJoinByEmailReplyWhenError = value; } }
        public static string GameHtmlReportsFolder { get { return gameHtmlReportsFolder; } set { gameHtmlReportsFolder = value; } }
        public static string GamePublicHtmlReportsFolder { get { return gamePublicHtmlReportsFolder; } set { gamePublicHtmlReportsFolder = value; } }
        public static string GameHtmlTimesFolder { get { return gameHtmlTimesFolder; } set { gameHtmlTimesFolder = value; } }
        public static bool GameRunTurnByEmail { get { return gameRunTurnByEmail; } set { gameRunTurnByEmail = value; } }
        public static string GameRunTurnByEmailPw { get { return gameRunTurnByEmailPw; } set { gameRunTurnByEmailPw = value; } }
        
        public static string IncMailServer { get { return incMailServer; } set { incMailServer = value; } }
        public static bool IncMailSSL { get { return incMailSSL; } set { incMailSSL = value; } }
        public static uint IncMailPort { get { return incMailPort; } set { incMailPort = value; } }
        public static string IncMailUser { get { return incMailUser; } set { incMailUser = value; } }
        public static string IncMailPw { get { return incMailPw; } set { incMailPw = value; } }
        public static bool IncMailLeaveCopyOnServer { get { return incMailLeaveCopyOnServer; } set { incMailLeaveCopyOnServer = value; } }

        public static string OutMailHost { get { return outMailHost; } set { outMailHost = value; } }
        public static bool OutMailSSL { get { return outMailSSL; } set { outMailSSL = value; } }
        public static uint OutMailPort { get { return outMailPort; } set { outMailPort = value; } }
        public static string OutMailUser { get { return outMailUser; } set { outMailUser = value; } }
        public static string OutMailPw { get { return outMailPw; } set { outMailPw = value; } }
        public static bool OutMailSendMail { get { return outMailSendMail; } set { outMailSendMail = value; } }

        public static bool GameSettingsFileExists() {
            return File.Exists(gameSettingsFile);
        }

        public static bool IncMailSettingsFileExists() {
            return File.Exists(incMailSettingsFile);
        }

        public static bool OutMailSettingsFileExists() {
            return File.Exists(outMailSettingsFile);
        }

        public static void LoadGameSettings() {
            if (File.Exists(gameSettingsFile)) {
                StreamReader sr = new StreamReader(gameSettingsFile);
                while (!sr.EndOfStream) {
                    string line = sr.ReadLine();
                    int idx;
                    string key;
                    if ((idx = line.IndexOf(key = "Name:")) > -1) {
                        gameName = line.Substring(idx + key.Length).Trim();
                    } else if ((idx = line.IndexOf(key = "JoinByEmail:")) > -1) {
                        gameJoinByEmail = (line.Substring(idx + key.Length).Trim().Equals("true")) ? true : false;
                    } else if ((idx = line.IndexOf(key = "JoinByEmailPw:")) > -1) {
                        gameJoinByEmailPw = line.Substring(idx + key.Length).Trim();
                    } else if ((idx = line.IndexOf(key = "JoinByEmailReplyWhenError:")) > -1) {
                        gameJoinByEmailReplyWhenError = (line.Substring(idx + key.Length).Trim().Equals("true")) ? true : false;
                    } else if ((idx = line.IndexOf(key = "PublicHtmlReportsFolder:")) > -1) {
                        gamePublicHtmlReportsFolder = line.Substring(idx + key.Length).Trim();
                    } else if ((idx = line.IndexOf(key = "HtmlReportsFolder:")) > -1) {
                        gameHtmlReportsFolder = line.Substring(idx + key.Length).Trim();
                    } else if ((idx = line.IndexOf(key = "HtmlTimesFolder:")) > -1) {
                        gameHtmlTimesFolder = line.Substring(idx + key.Length).Trim();
                    } else if ((idx = line.IndexOf(key = "RunTurnByEmail:")) > -1) {
                        gameRunTurnByEmail = (line.Substring(idx + key.Length).Trim().Equals("true")) ? true : false;
                    } else if ((idx = line.IndexOf(key = "RunTurnByEmailPw:")) > -1) {
                        gameRunTurnByEmailPw = line.Substring(idx + key.Length).Trim();
                    }
                }
                sr.Close();
            }
        }

        public static void SaveGameSettings() {
            StreamWriter sw = new StreamWriter(gameSettingsFile, false);
            sw.WriteLine("Name:" + gameName);
            sw.WriteLine("JoinByEmail:" + (gameJoinByEmail ? "true" : "false"));
            sw.WriteLine("JoinByEmailPw:" + gameJoinByEmailPw);
            sw.WriteLine("JoinByEmailReplyWhenError:" + (gameJoinByEmailReplyWhenError ? "true" : "false"));
            sw.WriteLine("HtmlReportsFolder:" + gameHtmlReportsFolder);
            sw.WriteLine("PublicHtmlReportsFolder:" + gamePublicHtmlReportsFolder);
            sw.WriteLine("HtmlTimesFolder:" + gameHtmlTimesFolder);
            sw.WriteLine("RunTurnByEmail:" + (gameRunTurnByEmail ? "true" : "false"));
            sw.WriteLine("RunTurnByEmailPw:" + gameRunTurnByEmailPw);
            sw.Close();
        }

        public static void LoadIncMailSettings() {
            if (File.Exists(incMailSettingsFile)) {
                StreamReader sr = new StreamReader(incMailSettingsFile);
                while (!sr.EndOfStream) {
                    string line = sr.ReadLine();
                    int idx;
                    string key;
                    if ((idx = line.IndexOf(key = "Server:")) > -1) {
                        incMailServer = line.Substring(idx + key.Length).Trim();
                    } else if ((idx = line.IndexOf(key = "SSL:")) > -1) {
                        incMailSSL = (line.Substring(idx + key.Length).Trim().Equals("true")) ? true : false;
                    } else if ((idx = line.IndexOf(key = "Port:")) > -1) {
                        incMailPort = Convert.ToUInt16(line.Substring(idx + key.Length).Trim());
                    } else if ((idx = line.IndexOf(key = "User:")) > -1) {
                        incMailUser = line.Substring(idx + key.Length).Trim();
                    } else if ((idx = line.IndexOf(key = "Password:")) > -1) {
                        incMailPw = line.Substring(idx + key.Length).Trim();
                    } else if ((idx = line.IndexOf(key = "Leave copy on server:")) > -1) {
                        incMailLeaveCopyOnServer = (line.Substring(idx + key.Length).Trim().Equals("true")) ? true : false;
                    }
                }
                sr.Close();
            }
        }

        public static void SaveIncMailSettings() {
            StreamWriter sw = new StreamWriter(incMailSettingsFile, false);
            sw.WriteLine("Server:" + incMailServer);
            sw.WriteLine("SSL:" + (incMailSSL ? "true" : "false"));
            sw.WriteLine("Port:" + incMailPort.ToString());
            sw.WriteLine("User:" + incMailUser);
            sw.WriteLine("Password:" + incMailPw);
            sw.WriteLine("Leave copy on server:" + (incMailLeaveCopyOnServer ? "true" : "false"));
            sw.Close();
        }

        public static void LoadOutMailSettings() {
            if (File.Exists(outMailSettingsFile)) {
                StreamReader sr = new StreamReader(outMailSettingsFile);
                while (!sr.EndOfStream) {
                    string line = sr.ReadLine();
                    int idx;
                    string key;
                    if ((idx = line.IndexOf(key = "Host:")) > -1) {
                        outMailHost = line.Substring(idx + key.Length).Trim();
                    } else if ((idx = line.IndexOf(key = "EnableSSL:")) > -1) {
                        outMailSSL = (line.Substring(idx + key.Length).Trim().Equals("true")) ? true : false;
                    } else if ((idx = line.IndexOf(key = "Port:")) > -1) {
                        outMailPort = Convert.ToUInt16(line.Substring(idx + key.Length).Trim());
                    } else if ((idx = line.IndexOf(key = "User:")) > -1) {
                        outMailUser = line.Substring(idx + key.Length).Trim();
                    } else if ((idx = line.IndexOf(key = "Password:")) > -1) {
                        outMailPw = line.Substring(idx + key.Length).Trim();
                    } else if ((idx = line.IndexOf(key = "Send Mail:")) > -1) {
                        outMailSendMail = (line.Substring(idx + key.Length).Trim().Equals("true")) ? true : false;
                    }
                }
                sr.Close();
            }
        }

        public static void SaveOutMailSettings() {
            StreamWriter sw = new StreamWriter(outMailSettingsFile, false);
            sw.WriteLine("Host:" + outMailHost);
            sw.WriteLine("EnableSSL:" + (outMailSSL ? "true" : "false"));
            sw.WriteLine("Port:" + outMailPort.ToString());
            sw.WriteLine("User:" + outMailUser);
            sw.WriteLine("Password:" + outMailPw);
            sw.WriteLine("Send Mail:" + (outMailSendMail ? "true" : "false"));
            sw.Close();
        }

    }

}
