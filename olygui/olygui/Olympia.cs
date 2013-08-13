using System;
using System.Text;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Windows.Forms;
using System.IO;
using System.Diagnostics;
using OpenPOP.POP3;
using OpenPOP.MIMEParser;
using mapgui;

namespace olygui {
    public partial class Olympia : Form {

        Timer timer = new Timer();
        DateTime timerStart = DateTime.Now;
        DateTime timerEnd = DateTime.Now;

        StringBuilder outputBuffer;
        bool runningProcess = false;

        static Random random;

        List<StartingCity> startingCities = new List<StartingCity>();


        public Olympia() {
            InitializeComponent();

            timer.Interval = 1000;
            timer.Tick += new EventHandler(timer_Tick);  
        }


        private void StartTimer() {
            tbAutoEmailMins.Enabled = false;
            btnMailSettings.Enabled = false;
            btnTestGmail.Enabled = false;
            btnSimTurn.Enabled = false;
            btnRunTurn.Enabled = false;
            btnGenerateMap.Enabled = false;
            btnMapEditor.Enabled = false;
            btnSetupGame.Enabled = false;
            btnCreateFaction.Enabled = false;
            timer.Start();
            lblAutoEmailStatus.Text = "Waiting";
            btnAutoEmailStartStop.Text = "Stop";
        }

        private void StopTimer() {
            timer.Stop();
            tbAutoEmailMins.Enabled = true;
            btnMailSettings.Enabled = true;
            btnTestGmail.Enabled = true;
            btnSimTurn.Enabled = true;
            btnRunTurn.Enabled = true;
            btnGenerateMap.Enabled = true;
            btnMapEditor.Enabled = true;
            btnSetupGame.Enabled = true;
            btnCreateFaction.Enabled = true;
            lblAutoEmailStatus.Text = "Not started";
            btnAutoEmailStartStop.Text = "Start";
        }

        private void timer_Tick(object sender, EventArgs e) {
            if (btnAutoEmailStartStop.Text.Equals("Start")) {
                StopTimer();
            } else { 
                DateTime now = DateTime.Now;
                TimeSpan span = timerEnd - now;
                if (span.TotalSeconds <= 0) {
                    // we need to check the mail
                    timer.Stop();
                    lblAutoEmailStatus.Text = "Checking E-mail";
                    btnAutoEmailStartStop.Enabled = false;
                    CheckEmail();
                    btnAutoEmailStartStop.Enabled = true;
                    timerStart = DateTime.Now;
                    double mins = Convert.ToDouble(tbAutoEmailMins.Text);
                    timerEnd = timerStart.AddMinutes(mins);
                    StartTimer();
                }
                tbAutoEmailNextCheck.Text = span.Minutes.ToString() + ":" + span.Seconds.ToString("0#");
            }
        }

        private void Log(string str) {
            tbOutput.AppendText(str);
            DateTime now = DateTime.Now;
            string dateStr = now.Year.ToString() + "_" + now.Month.ToString() + "_" + now.Day.ToString();
            StreamWriter log = new StreamWriter(Directory.GetCurrentDirectory() + "\\" + dateStr + ".log", true);
            string d = DateTime.Now.ToString();
            log.Write(d + "\t" + str);
            log.Close();
        }
        private void Logn(string str) {
            Log(str + Environment.NewLine);
        }

        private void btnSetupGame_Click(object sender, EventArgs e) {
            if (MessageBox.Show("Warning, this will erase all game data in this folder.  Are you sure you want to set up a new game?", "Setup new game", MessageBoxButtons.YesNo) == DialogResult.Yes) {

                lbQueuedFactions.Items.Clear();

                // store paths
                string curdir = Directory.GetCurrentDirectory();
                string libdir = curdir + "\\lib";

                // delete the lib dir if it exists
                if (Directory.Exists(libdir)) {
                    Log("Deleting old database...");
                    Directory.Delete(libdir, true);
                    Logn("done.");
                }

                // unzip the original lib folder
                Log("Installing new database...");
                cZip.UnZipFiles(curdir + "\\lib_orig.zip", curdir, "", false);
                Logn("done.");

                // Add a few necessary folders
                if (Directory.Exists(curdir + "\\backup")) {
                    Log("Deleting old backup folder...");
                    Directory.Delete(curdir + "\\backup", true);
                    Logn("done.");
                }
                Log("Creating backup folder...");
                Directory.CreateDirectory(curdir + "\\backup");
                Logn("done.");

                if (Directory.Exists(curdir + "\\conf")) {
                    Log("Deleting old conf folder...");
                    Directory.Delete(curdir + "\\conf", true);
                    Logn("done.");
                }
                Log("Creating conf folder...");
                Directory.CreateDirectory(curdir + "\\conf");
                Logn("done.");

                if (Directory.Exists(curdir + "\\act")) {
                    Log("Deleting old act folder...");
                    Directory.Delete(curdir + "\\act", true);
                    Logn("done.");
                }
                Log("Creating act folder...");
                Directory.CreateDirectory(curdir + "\\act");
                Logn("done.");

                if (Directory.Exists(curdir + "\\log")) {
                    Log("Deleting old log folder...");
                    Directory.Delete(curdir + "\\log", true);
                    Logn("done.");
                }
                Log("Creating log folder...");
                Directory.CreateDirectory(curdir + "\\log");
                Logn("done.");

                if (Directory.Exists(curdir + "\\old-act")) {
                    Log("Deleting old old-act folder...");
                    Directory.Delete(curdir + "\\old-act", true);
                    Logn("done.");
                }
                Log("Creating olg-act folder...");
                Directory.CreateDirectory(curdir + "\\old-act");
                Logn("done.");

                if (Directory.Exists(curdir + "\\rep")) {
                    Log("Deleting old rep folder...");
                    Directory.Delete(curdir + "\\rep", true);
                    Logn("done.");
                }
                Log("Creating rep folder...");
                Directory.CreateDirectory(curdir + "\\rep");
                Logn("done.");

                if (Directory.Exists(curdir + "\\public")) {
                    Log("Deleting old public folder...");
                    Directory.Delete(curdir + "\\public", true);
                    Logn("done.");
                }
                Log("Creating public folder...");
                Directory.CreateDirectory(curdir + "\\public");
                Logn("done.");

                if (Directory.Exists(curdir + "\\tmp")) {
                    Log("Deleting old tmp folder...");
                    Directory.Delete(curdir + "\\tmp", true);
                    Logn("done.");
                }
                Log("Creating tmp folder...");
                Directory.CreateDirectory(curdir + "\\tmp");
                Logn("done.");

                if (Directory.Exists(libdir + "\\spool")) {
                    Log("Deleting old lib\\spool folder...");
                    Directory.Delete(libdir + "\\spool", true);
                    Logn("done.");
                }
                Log("Creating lib\\spool folder...");
                Directory.CreateDirectory(libdir + "\\spool");
                Logn("done.");

                if (Directory.Exists(libdir + "\\orders")) {
                    Log("Deleting old lib\\orders folder...");
                    Directory.Delete(libdir + "\\orders", true);
                    Logn("done.");
                }
                Log("Creating lib\\orders folder...");
                Directory.CreateDirectory(libdir + "\\orders");
                Logn("done.");

                if (Directory.Exists(libdir + "\\html")) {
                    Log("Deleting old lib\\html folder...");
                    Directory.Delete(libdir + "\\html", true);
                    Logn("done.");
                }
                Log("Creating lib\\html folder...");
                Directory.CreateDirectory(libdir + "\\html");
                Logn("done.");

                Log("Generating act\\.alloc file...");
                StreamWriter actAlloc = new StreamWriter(curdir + "\\act\\.alloc");
                string letters = "abcdefghijklmnopqrstuvwxyz";
                int i, j, k;
                for (i = 0; i < 26; i++)
                    for (j = 0; j < 26; j++)
                        for (k = 0; k < 10; k++)
                            actAlloc.WriteLine(letters[i].ToString() + letters[j].ToString() + k.ToString());
                actAlloc.Close();
                Logn("done.");

                string mapdir = curdir + "\\mapgen";

                // Checking whether we need to generate a new map or not
                if (Directory.Exists(mapdir)) {
                    // running mapgen if there's no loc, road or gate file in \\mapgen
                    if (!File.Exists(mapdir + "\\loc") || !File.Exists(mapdir + "\\gate") || !File.Exists(mapdir + "\\road")) {
                        Logn("Generating map:");
                        Logn(" -- working, please wait --");
                        Log(RunProcess(mapdir + "\\mapgen.exe", ""));
                        Logn(" -- done --");
                    }

                    // Check if we have the files now
                    if (!File.Exists(mapdir + "\\loc") || !File.Exists(mapdir + "\\gate") || !File.Exists(mapdir + "\\road")) {
                        Logn("An error occurred during the generation of the map.");
                        return;
                    } else {
                        Log("Copying map files...");
                        File.Delete(libdir + "\\loc");
                        File.Move(mapdir + "\\loc", libdir + "\\loc");
                        File.Delete(libdir + "\\gate");
                        File.Move(mapdir + "\\gate", libdir + "\\gate");
                        File.Delete(libdir + "\\road");
                        File.Move(mapdir + "\\road", libdir + "\\road");
                        Logn("done.");
                    }
                }

                Logn("Starting Olympia setup:");
                Logn(" -- working, please wait --");
                Log(RunProcess(curdir + "\\Olympia2.exe", "-w -S"));
                Logn(" -- done --");

                // We need to read the lib\loc file and find the starting city/safe haven data
                Log("Finding starting location data...");
                string locfile = libdir + "\\loc";
                List<string> block = new List<string>();
                Dictionary<string, StartingCity> sc = new Dictionary<string, StartingCity>();
                if (File.Exists(locfile)) {
                    StreamReader locreader = new StreamReader(locfile);
                    bool sh = false;
                    int loc_city = -1;
                    string cname = "";
                    int idx = 0;
                    while (!locreader.EndOfStream) {
                        string line = locreader.ReadLine();
                        int t = -1;
                        if ((t = line.ToLower().IndexOf(" loc city")) > -1) {
                            loc_city = Convert.ToInt32(line.Substring(0, t));
                        } else if (line.ToLower().IndexOf(" sh ") > -1) {
                            sh = true;
                        } else if ((t = line.ToLower().IndexOf("na ")) == 0) {
                            cname = line.Substring(3);
                        } else if (line.Trim().Equals("")) {
                            if (loc_city > -1 && sh) {
                                StartingCity c = new StartingCity(idx++, loc_city, cname);
                                sc.Add(cname, c);
                            }
                            block = new List<string>();
                            loc_city = -1;
                            sh = false;
                            cname = "";
                        } else {
                            block.Add(line);
                        }
                    }
                    locreader.Close();
                }
                startingCities = new List<StartingCity>();
                if (File.Exists(mapdir + "\\Havens")) {
                    StreamReader havenfile = new StreamReader(mapdir + "\\Havens");
                    while (!havenfile.EndOfStream) {
                        string line = havenfile.ReadLine();
                        if (!line.Trim().Equals(""))
                            if (sc.ContainsKey(line.Substring(2)))
                                startingCities.Add(sc[line.Substring(2)]);
                    }
                    havenfile.Close();
                } else {
                    foreach (StartingCity s in sc.Values)
                        startingCities.Add(s);
                }
                UpdateStartingCities();
                Logn("done.");

                // write these starting locations to a file
                Log("Updating starting locations...");
                StreamWriter shfile = new StreamWriter(libdir + "\\startloc", false);
                foreach (StartingCity c in startingCities) {
                    shfile.WriteLine(c.idx.ToString() + " " + c.id.ToString() + " " + c.name);
                }
                shfile.Close();
                Logn("done.");

                UpdateCurrentTurn();
            }
        }

        private string RunProcess(string file, string arguments) {
            string retval = "";
            if (!runningProcess) {
                runningProcess = true;
                this.Cursor = Cursors.WaitCursor;
                Process proc = new Process();
                proc.StartInfo.FileName = file;
                proc.StartInfo.Arguments = arguments;
                proc.StartInfo.UseShellExecute = false;
                proc.StartInfo.CreateNoWindow = true;
                proc.StartInfo.WorkingDirectory = Directory.GetParent(file).ToString();
//                proc.StartInfo.ErrorDialog = false;
                proc.StartInfo.RedirectStandardOutput = true;
                proc.StartInfo.RedirectStandardError = true;
                outputBuffer = new StringBuilder("");
                proc.OutputDataReceived += new DataReceivedEventHandler(process_outputreceived);
                proc.ErrorDataReceived += new DataReceivedEventHandler(process_outputreceived);
                proc.Start();
                proc.BeginOutputReadLine();
                proc.BeginErrorReadLine();
                proc.WaitForExit();
                retval = outputBuffer.ToString();
                proc.Close();
                this.Cursor = Cursors.Default;
                runningProcess = false;
            } else {
                MessageBox.Show("I'm busy, please try again in a little while.");
            }
            return retval;
        }

        private void process_outputreceived(object sender, DataReceivedEventArgs args) {
            if (!String.IsNullOrEmpty(args.Data)) {
                outputBuffer.Append(args.Data + Environment.NewLine);
            }
        }

        private void Olympia_Load(object sender, EventArgs e) {
            random = new Random();

            /** 
             * According to oly code:
             * 1: ID# 57140 Drassa
             * 2: ID# 58736 Rimmon
             * 3: ID# 57081 Greyfell
             * 4: ID# 58423 Port Aurnos
             * 5: ID# 58112 Yellowleaf
             * 6: ID# 58335 Harn 
             **/
            Settings.LoadGameSettings();
            //Dictionary<string, string> gameSettings = GameSettings.LoadGameSettings();
            this.Text = Settings.GameName;
            LoadStartLocations();
            UpdateStartingCities();
            UpdateCurrentTurn();
        }

        private void LoadStartLocations() { 
            // if lib\startloc exists, load the starting locations
            string curdir = Directory.GetCurrentDirectory();
            string libdir = curdir + "\\lib";
            string scfile = libdir + "\\startloc";
            if (File.Exists(scfile)) {
                startingCities = new List<StartingCity>();
                StreamReader sr = new StreamReader(scfile);
                while (!sr.EndOfStream) {
                    string line = sr.ReadLine();
                    if (!line.Trim().Equals("") && line.Length > 9) {
                        StartingCity c = new StartingCity();
                        c.idx = Convert.ToInt32(line.Substring(0, 1));
                        c.id = Convert.ToInt32(line.Substring(2, 5));
                        c.name = line.Substring(8);
                        startingCities.Add(c);
                    }
                }
                sr.Close();
            } else {
                string locfile = libdir + "\\loc";
                List<string> block = new List<string>();
                Dictionary<string, StartingCity> sc = new Dictionary<string, StartingCity>();
                if (File.Exists(locfile)) {
                    StreamReader locreader = new StreamReader(locfile);
                    bool sh = false;
                    int loc_city = -1;
                    string cname = "";
                    int idx = 0;
                    while (!locreader.EndOfStream) {
                        string line = locreader.ReadLine();
                        int t = -1;
                        if ((t = line.ToLower().IndexOf(" loc city")) > -1) {
                            loc_city = Convert.ToInt32(line.Substring(0, t));
                        } else if (line.ToLower().IndexOf(" sh ") > -1) {
                            sh = true;
                        } else if ((t = line.ToLower().IndexOf("na ")) == 0) {
                            cname = line.Substring(3);
                        } else if (line.Trim().Equals("")) {
                            if (loc_city > -1 && sh) {
                                StartingCity c = new StartingCity(idx++, loc_city, cname);
                                sc.Add(cname, c);
                            }
                            block = new List<string>();
                            loc_city = -1;
                            sh = false;
                            cname = "";
                        } else {
                            block.Add(line);
                        }
                    }
                    locreader.Close();
                }
                startingCities = new List<StartingCity>();
                string mapdir = curdir + "\\mapgen";
                if (File.Exists(mapdir + "\\Havens")) {
                    StreamReader havenfile = new StreamReader(mapdir + "\\Havens");
                    while (!havenfile.EndOfStream) {
                        string line = havenfile.ReadLine();
                        if (!line.Trim().Equals(""))
                            if (sc.ContainsKey(line.Substring(2)))
                                startingCities.Add(sc[line.Substring(2)]);
                    }
                    havenfile.Close();
                }

            }
        }

        private void UpdateStartingCities() {
            cbStartingCity.Items.Clear();
            cbStartingCity.Items.AddRange(startingCities.ToArray());
            if (cbStartingCity.Items.Count > 0)
                cbStartingCity.SelectedIndex = 0;
        }

        private void btnCreateFaction_Click(object sender, EventArgs e) {
            string curdir = Directory.GetCurrentDirectory();
            // check if all information is filled in
            if (!tbFactionName.Text.Trim().Equals("")
                && !tbNobleName.Text.Trim().Equals("")
                && !tbPlayerName.Text.Trim().Equals("")
                && !tbPlayerEmail.Text.Trim().Equals("")) {
                AddFaction(tbFactionName.Text,
                tbNobleName.Text,
                ((StartingCity)cbStartingCity.SelectedItem).name,
                tbPlayerName.Text,
                tbPlayerEmail.Text);
            } else {
                MessageBox.Show("Not all faction information is filled in.");
            }
        }

        private void btnAddQueuedFactionsToGame_Click(object sender, EventArgs e) {
            string curdir = Directory.GetCurrentDirectory(); 
            this.Cursor = Cursors.WaitCursor;
            Logn("Adding queued factions to game:");
            Logn(" -- working, please wait --");
            try {
                Log(RunProcess(curdir + "\\Olympia2.exe", "-w -S -a -M"));
            } catch (Exception ex) {
                Logn(ex.Message);
            }
            lbQueuedFactions.Items.Clear();
            Logn(" -- done --");
            Logn("Initial turn reports can be found in " + curdir + "\\rep\\");
            this.Cursor = Cursors.Default;
        }

        private void btnMailSettings_Click(object sender, EventArgs e) {
            MailSettings form = new MailSettings();
            form.ShowDialog();
        }

        private void UpdateCurrentTurn() {
            tbCurrentTurn.Text = GetTurn().ToString();
        }

        private void CreateTimes() {
            Settings.LoadIncMailSettings();
            string curdir = Directory.GetCurrentDirectory();
            string libdir = curdir + "\\lib";
            string timesdir = curdir + "\\times";
            // First check if times dir exists
            if (!Directory.Exists(timesdir))
                Directory.CreateDirectory(timesdir);
            string fname = timesdir + "\\times_" + GetTurn().ToString() + ".txt";
            StreamWriter sw = new StreamWriter(fname, false);
            string t = libdir + "\\times_gm";
            StreamReader sr;
            if (File.Exists(t)) {
                sr = new StreamReader(t);
                if (!sr.EndOfStream)
                    sw.Write(sr.ReadToEnd());
                sr.Close();
            }
            t = libdir + "\\times_0";
            if (File.Exists(t)) {
                sr = new StreamReader(t);
                if (!sr.EndOfStream) {
                    sr.ReadLine();
                    sr.ReadLine();
                    sr.ReadLine();
                    string txt = sr.ReadToEnd();
                    int mlen = Settings.IncMailUser.Length;
                    string mailtxt = Settings.IncMailUser.PadLeft(48, ' ');
                    sw.Write(txt.Replace("!game_email!", mailtxt));
                }
                sr.Close();
            }
            t = libdir + "\\times_press";
            if (File.Exists(t)) {
                sr = new StreamReader(t);
                if (!sr.EndOfStream)
                    sw.Write(sr.ReadToEnd());
                sr.Close();
            }
            t = libdir + "\\times_rumor";
            if (File.Exists(t)) {
                sr = new StreamReader(t);
                if (!sr.EndOfStream)
                    sw.Write(sr.ReadToEnd());
                sr.Close();
            }
            sw.Close();
        }

        private void MailTimes() {
            Settings.LoadIncMailSettings();
            Settings.LoadOutMailSettings();
            if (Settings.OutMailSendMail) {
                string curdir = Directory.GetCurrentDirectory();
                string libdir = curdir + "\\lib";
                string timesdir = curdir + "\\times";
                List<string> emails = new List<string>();
                string fname = libdir + "\\totimes";
                if (File.Exists(fname)) {
                    StreamReader sr = new StreamReader(fname);
                    while (!sr.EndOfStream) {
                        string line = sr.ReadLine().Trim();
                        if (!emails.Contains(line))
                            emails.Add(line);
                    }
                    sr.Close();
                    foreach (string email in emails) { 
                        string tname = curdir + "\\tmp\\times";
                        StreamWriter sw = new StreamWriter(tname);
                        sw.WriteLine("From: " + Settings.IncMailUser);
                        sw.WriteLine("Reply-To: " + Settings.IncMailUser);
                        sw.WriteLine("To: " + email);
                        sw.WriteLine("Subject: The Olympia Times - Issue " + GetTurn().ToString());
                        sw.WriteLine("");
                        string times = timesdir + "\\times_" + GetTurn().ToString() + ".txt";
                        StreamReader timesfile = new StreamReader(times);
                        if (!timesfile.EndOfStream)
                            sw.Write(timesfile.ReadToEnd());
                        timesfile.Close();
                        sw.Close();
                        // send it
                        try {
                            RunProcess("sendmail", tname);
                        } catch (Exception ex) {
                            Logn(ex.Message);
                        }
                        Logn(" -- sent to " + email);
                        File.Delete(tname);
                    }
                }
            }
        }

        private void RunTurn() {
            string curdir = Directory.GetCurrentDirectory();
            Log("Backing up previous turn data...");
            backupTurn();
            Logn("done.");
            int bturn = GetTurn();
            Logn("Running turn:");
            Logn(" -- working, please wait --");
            Log(RunProcess(curdir + "\\Olympia2.exe", "-w -r -M -h -S"));
            Logn(" -- done --");
            int aturn = GetTurn();
            if (aturn != bturn) {
                Logn("Sending The Olympia Times:");
                CreateTimes();
                MailTimes();
                Logn("Done.");
                Settings.LoadGameSettings();
                // HTML reports
                if (Settings.GameHtmlReportsFolder != curdir + "\\lib\\html") {
                    Log("Moving HTML reports...");
                    string destFolder = Settings.GameHtmlReportsFolder + "\\" + aturn;
                    if (!Directory.Exists(destFolder))
                        Directory.CreateDirectory(destFolder);
                    foreach (string dir in Directory.GetDirectories(curdir + "\\lib\\html")) {
                        string[] parts = dir.Split('\\');
                        if (parts.Length > 0) {
                            string d = parts[parts.Length - 1];
                            if (!Directory.Exists(destFolder + "\\" + d))
                                Directory.CreateDirectory(destFolder + "\\" + d);
                            foreach (string file in Directory.GetFiles(dir)) {
                                parts = file.Split('\\');
                                if (parts.Length > 0) {
                                    string f = parts[parts.Length - 1];
                                    File.Copy(dir + "\\" + f, destFolder + "\\" + d + "\\" + f, true);
                                }
                            }
                            // Overwrite .htaccess with own pw stuff
                            StreamWriter hta = new StreamWriter(destFolder + "\\" + d + "\\.htaccess", false);
                            hta.WriteLine("AuthType Basic");
                            hta.WriteLine("AuthName \"Password for faction " + d + "\"");
                            hta.WriteLine("AuthUserFile \"" + curdir + "\\lib\\.htpasswd\"");
                            hta.WriteLine("<Limit GET POST>");
                            hta.WriteLine("require user " + d);
                            hta.WriteLine("</Limit>");
                            hta.WriteLine("DirectoryIndex index.html");
                            hta.Close();
                            // Copy the head.gif image file
                            string imgfile = curdir + "\\head.gif";
                            File.Copy(imgfile, destFolder + "\\" + d + "\\head.gif", true);
                        }
                    }
                    Logn("Done.");
                }
                // Public turn reports
                if (Settings.GamePublicHtmlReportsFolder != curdir + "\\public") {
                    Log("Moving Public HTML reports...");
                    string publicContents = "";
                    if (File.Exists(Settings.GamePublicHtmlReportsFolder + "\\pubidx.inc")) {
                        StreamReader baseIdxFileOrig = new StreamReader(Settings.GamePublicHtmlReportsFolder + "\\pubidx.inc");
                        publicContents = baseIdxFileOrig.ReadToEnd();
                        baseIdxFileOrig.Close();
                    }
                    StreamWriter baseIdxFile = new StreamWriter(Settings.GamePublicHtmlReportsFolder + "\\pubidx.inc", false);
                    string[] reps = Directory.GetFiles(curdir + "\\public");
                    foreach (string rep in reps) {
                        string[] parts = rep.Split('\\');
                        if (parts.Length > 0) {
                            string f = parts[parts.Length - 1];
                            StreamReader orig = new StreamReader(rep);
                            string dest = Settings.GamePublicHtmlReportsFolder + "\\" + aturn + "_" + f;
                            StreamWriter cpy = new StreamWriter(dest);
                            while (!orig.EndOfStream) {
                                string line = orig.ReadLine();
                                if (line.Trim().ToLower().IndexOf("order template") > -1)
                                    break;
                                cpy.WriteLine(line);
                            }
                            cpy.Close();
                            orig.Close();
                            baseIdxFile.WriteLine("<a href=\"" + aturn + "_" + f + "\">Turn " + aturn + " - " + f.Replace(".html", "") + "</a><br />");
                            File.Delete(rep);
                        }
                    }
                    baseIdxFile.Write(publicContents);
                    baseIdxFile.Close();
                    if (publicContents == "" && reps.Length == 0)
                        File.Delete(Settings.GamePublicHtmlReportsFolder + "\\pubidx.inc");
                    else {
                        // Copy the head.gif image file
                        string imgfile = curdir + "\\head.gif";
                        File.Copy(imgfile, Settings.GamePublicHtmlReportsFolder + "\\head.gif", true);
                    }
                    Logn("Done.");
                }
                // html times
                if (Settings.GameHtmlTimesFolder != curdir + "\\times") {
                    Log("Moving The Olympia Times...");
                    string file = curdir + "\\times\\times_" + aturn + ".txt";
                    StreamReader timesFile = new StreamReader(file);
                    string timesContents = "";
                    if (!timesFile.EndOfStream)
                        timesContents = timesFile.ReadToEnd();
                    timesFile.Close();
                    if (timesContents != "") {
                        string newTimesContents = 
                            "<html>" + Environment.NewLine + 
                            " <head>" + Environment.NewLine +
                            "  <title>" + Settings.GameName + " - The Olympia Times - Issue " + aturn + "</title>" + Environment.NewLine +
                            " </head>" + Environment.NewLine + 
                            " <body>" + Environment.NewLine +
                            "  <pre>" + Environment.NewLine +
                            timesContents + Environment.NewLine +
                            "  </pre>" + Environment.NewLine +
                            " </body>" + Environment.NewLine +
                            "</html>";
                        StreamWriter newTimesFile = new StreamWriter(Settings.GameHtmlTimesFolder + "\\t" + aturn + ".html");
                        newTimesFile.Write(newTimesContents);
                        newTimesFile.Close();
                        // Times index file
                        string idxFileContents = "";
                        if (File.Exists(Settings.GameHtmlTimesFolder + "\\timesidx.inc")) {
                            StreamReader idxFile = new StreamReader(Settings.GameHtmlTimesFolder + "\\timesidx.inc");
                            idxFileContents = idxFile.ReadToEnd();
                            idxFile.Close();
                        }
                        StreamWriter idxFile2 = new StreamWriter(Settings.GameHtmlTimesFolder + "\\timesidx.inc", false);
                        idxFile2.WriteLine("<a href=\"t" + aturn + ".html\">Turn " + aturn + "</a><br />");
                        idxFile2.Write(idxFileContents);
                        idxFile2.Close();
                    }
                    Logn("Done.");
                }
            }
            lbQueuedFactions.Items.Clear();
            UpdateCurrentTurn();
        }

        private void btnRunTurn_Click(object sender, EventArgs e) {
            if (MessageBox.Show("You are about to run a turn.  Are you sure?", "Run Turn", MessageBoxButtons.YesNo) == DialogResult.Yes)
                RunTurn();
        }

        private int GetTurn() {
            int retval = 0;
            string curdir = Directory.GetCurrentDirectory();
            string filename = curdir + "\\lib\\system";
            if (File.Exists(filename)) {
                StreamReader sr = new StreamReader(curdir + "\\lib\\system");
                while (!sr.EndOfStream) {
                    string line = sr.ReadLine();
                    if (line.IndexOf("sysclock:") > -1) {
                        line = line.Substring(9).Trim();
                        int space = line.IndexOf(' ');
                        retval = Convert.ToInt32(line.Substring(0, space));
                        break;
                    }
                }
                sr.Close();
            }
            return retval;
        }

        private void backupTurn() {
            string curdir = Directory.GetCurrentDirectory();
            if (Directory.Exists(curdir + "\\backup")) {
                int turn = GetTurn();
                // first back up all folders
                // act folder
                if (Directory.Exists(curdir + "\\act"))
                    cZip.ZipFolder(curdir + "\\act", curdir + "\\backup\\" + turn.ToString() + "_act.zip", "");
                // lib folder
                if (Directory.Exists(curdir + "\\lib"))
                    cZip.ZipFolder(curdir + "\\lib", curdir + "\\backup\\" + turn.ToString() + "_lib.zip", "");
                // log folder
                if (Directory.Exists(curdir + "\\log"))
                    cZip.ZipFolder(curdir + "\\log", curdir + "\\backup\\" + turn.ToString() + "_log.zip", "");
                // old-act folder
                if (Directory.Exists(curdir + "\\old-act"))
                    cZip.ZipFolder(curdir + "\\old-act", curdir + "\\backup\\" + turn.ToString() + "_old-act.zip", "");
                // rep folder
                if (Directory.Exists(curdir + "\\rep"))
                    cZip.ZipFolder(curdir + "\\rep", curdir + "\\backup\\" + turn.ToString() + "_rep.zip", "");
                // tmp folder
                if (Directory.Exists(curdir + "\\tmp"))
                    cZip.ZipFolder(curdir + "\\tmp", curdir + "\\backup\\" + turn.ToString() + "_tmp.zip", "");
            }
        }

        private void AddFaction(string name, string noble, string city, string player, string email) {
            string curdir = Directory.GetCurrentDirectory();
            this.Cursor = Cursors.WaitCursor;
            Log("Adding faction...");
            // pick random faction code from act\.alloc file
            List<string> factionCodes = new List<string>();
            if (File.Exists(curdir + "\\act\\.alloc")) {
                StreamReader sr = new StreamReader(curdir + "\\act\\.alloc");
                while (!sr.EndOfStream)
                    factionCodes.Add(sr.ReadLine());
                sr.Close();
            }
            int max = factionCodes.Count;
            int rnd = random.Next(max);
            string newCode = factionCodes[rnd].Trim();
            Log(newCode + "...");
            factionCodes.RemoveAt(rnd);
            // create dir with faction code under act\
            Directory.CreateDirectory(curdir + "\\act\\" + newCode);
            // create file in that dir names Join-g3 with contents:
            // faction name
            // noble name
            // starting city
            // player name
            // player email
            StreamWriter sw = new StreamWriter(curdir + "\\act\\" + newCode + "\\Join-g3");
            sw.WriteLine(name.Trim());
            sw.WriteLine(noble.Trim());
            // checking if a random starting city was chosen
            int iCity = 1;
            if (city.ToLower().Trim().Equals("random")) {
                iCity = random.Next(0, startingCities.Count);
            } else {
                foreach (StartingCity c in startingCities) {
                    if (c.name.ToLower().Trim().Equals(city.ToLower().Trim())) {
                        iCity = c.idx;
                        break;
                    }
                }
            }
            sw.WriteLine(iCity.ToString());
            sw.WriteLine(player.Trim());
            sw.WriteLine(email.Trim());
            sw.Close();
            // update act\.alloc file
            sw = new StreamWriter(curdir + "\\act\\.alloc", false);
            foreach (string code in factionCodes)
                sw.WriteLine(code);
            sw.Close();
            // Update the to-add faction list
            lbQueuedFactions.Items.Add(newCode);
            Logn("done.");
            this.Cursor = Cursors.Default;
        }

        private void AddEmailToTempList(string email) {
            string curdir = Directory.GetCurrentDirectory();
            string fname = curdir + "\\newemails";
            StreamWriter sw = new StreamWriter(fname, true);
            sw.WriteLine(email.Trim());
            sw.Close();
        }

        private bool IsEmailInList(string email, string filename) {
            bool retval = false;
            if (File.Exists(filename)) {
                StreamReader sr = new StreamReader(filename);
                while (!sr.EndOfStream && !retval) {
                    string line = sr.ReadLine().Trim();
                    if (line.Equals(email.Trim()))
                        retval = true;
                }
                sr.Close();
            }
            return retval;
        }

        private bool IsEmailInGameList(string email) {
            string curdir = Directory.GetCurrentDirectory();
            string fname = curdir + "\\lib\\email";
            return IsEmailInList(email, fname);
        }

        private bool IsEmailInTempList(string email) {
            // check if the temp list exists
            string curdir = Directory.GetCurrentDirectory();
            string fname = curdir + "\\newemails";
            if (File.Exists(fname))
                return IsEmailInList(email, fname);
            return false;
        }

        private void CheckEmail() {
            string curdir = Directory.GetCurrentDirectory();
            this.Cursor = Cursors.WaitCursor;
            bool adminRunTurn = false;
            if (!Settings.IncMailSettingsFileExists()) {
                MessageBox.Show(this, "Incoming e-mail settings not found.  Please click \"E-mail Settings\" to configure your incoming e-mail account.");
            } else {
                Settings.LoadGameSettings();
                Settings.LoadIncMailSettings(); 
                Logn("Checking E-mails:");
                POPClient popClient = new POPClient();
                try {
                    popClient.Connect(Settings.IncMailServer, (int)Settings.IncMailPort, Settings.IncMailSSL);
                    popClient.Authenticate(Settings.IncMailUser, Settings.IncMailPw);
                    int Count = popClient.GetMessageCount();
                    Logn("Found " + Count.ToString() + " message(s).");
                    for (int i = Count; i >= 1; i -= 1) {
                        OpenPOP.MIMEParser.Message m = popClient.GetMessage(i, false);
                        Log("Message " + i.ToString() + " ");
                        if (m != null) {
                            string body = "";
                            if (m.ContentType == null || m.ContentType.Equals("text/plain")) {
                                body = m.RawMessageBody;
                            } else {
                                for (int j = 0; j < m.AttachmentCount; j++) {
                                    Attachment at = (Attachment)m.Attachments[j];
                                    if (at.ContentType.Equals("text/plain")) {
                                        body += at.DecodeAsText();
                                    }
                                }
                            }
                            string adminRunTurnCmd = "#admin_run_turn";
                            string diploForwardCmd = "#forwardto:";
                            string joinCmd1 = "#join:";
                            string joinCmd2 = "#join (hidden):";
                            // Diplomatic forwarding
                            if (body.IndexOf(diploForwardCmd) > -1) {
                                Log("seems to be a diplomatic forwarding message...");
                                string[] parts = body.Split('\n');
                                int idx;
                                string emailto = "";
                                StringBuilder msg = null;
                                for (int j = 0; j < parts.Length; j++) {
                                    if (emailto == "") {
                                        if ((idx = parts[j].IndexOf(diploForwardCmd)) > -1) {
                                            string fwto = parts[j].Substring(idx + diploForwardCmd.Length).Trim();
                                            if (fwto.Length == 3 || fwto.Length == 4) {
                                                //  faction                noble
                                                // Load forward file
                                                string fwfile = curdir + "\\lib\\forward";
                                                if (File.Exists(fwfile)) {
                                                    StreamReader sr = new StreamReader(fwfile);
                                                    while (!sr.EndOfStream) {
                                                        string line = sr.ReadLine();
                                                        string[] parts2 = line.Split('|');
                                                        if (parts2.Length == 2 && parts2[0] == fwto) {
                                                            emailto = parts2[1];
                                                            break;
                                                        }
                                                    }
                                                    sr.Close();
                                                } else {
                                                    Logn("forward file missing.  Abandoned.");
                                                    break;
                                                }
                                            } else {
                                                Logn("invalid forwarding id.  Abandoned.");
                                                break;
                                            }
                                        }
                                    } else {
                                        if (msg == null) {
                                            if (!parts[j].Trim().Equals("")) {
                                                msg = new StringBuilder(parts[j]);
                                            }
                                        } else {
                                            msg.Append(parts[j]);
                                        }
                                    }

                                }
                                string subj;
                                string to;
                                string from;
                                string txt;
                                if (emailto != "") {
                                    subj = "Diplomatic Message";
                                    to = emailto;
                                    if (m.ReplyToEmail != null)
                                        from = m.ReplyToEmail;
                                    else
                                        from = m.FromEmail;
                                    txt = "From: " + from + Environment.NewLine +
                                        "Reply-To: " + from + Environment.NewLine +
                                        "To: " + to + Environment.NewLine +
                                        "Subject: " + subj + Environment.NewLine + Environment.NewLine;
                                    string fromtxt = m.FromEmail;
                                    if (!m.From.Trim().Equals(""))
                                        fromtxt += " (" + m.From + ")";
                                    txt += "A diplomatic message has been sent to you by " + fromtxt + "." + Environment.NewLine
                                        + "It reads:" + Environment.NewLine + Environment.NewLine;
                                    txt += msg.ToString();
                                    string fname = Olympia.UniqueFileName();
                                    StreamWriter sw = new StreamWriter(fname);
                                    sw.Write(txt);
                                    sw.Close();
                                    Logn("forwarding it:");
                                    Logn(" -- working, please wait --");
                                    Logn(RunProcess("sendmail", fname));
                                    Logn(" -- done --");
                                    File.Delete(fname);
                                }
                                // do not tell the originating e-mailer that a match was found
                                // leave it open to avoid misuse
                                subj = "Re: " + m.Subject;
                                to = "";
                                if (m.ReplyToEmail != null)
                                    to = m.ReplyToEmail;
                                else
                                    to = m.FromEmail;
                                txt = "From: " + Settings.IncMailUser + Environment.NewLine +
                                    "Reply-To: " + Settings.IncMailUser + Environment.NewLine +
                                    "To: " + to + Environment.NewLine +
                                    "Subject: " + subj + Environment.NewLine + Environment.NewLine;
                                txt += "Your diplomatic message was well received." + Environment.NewLine + Environment.NewLine
                                    + "If a match is found, your message will be forwarded.  Please be aware that we cannot provide you with information of whether it is actually being sent or received." + Environment.NewLine + Environment.NewLine
                                    + "Your original message was:" + Environment.NewLine + Environment.NewLine;
                                txt += body;
                                string fname2 = Olympia.UniqueFileName();
                                StreamWriter sw2 = new StreamWriter(fname2);
                                sw2.Write(txt);
                                sw2.Close();
                                Logn("Also sending confirmation e-mail to sender:");
                                Logn(" -- working, please wait --");
                                Logn(RunProcess("sendmail", fname2));
                                Logn(" -- done --");
                                File.Delete(fname2);
                            // Admin command
                            } else if (body.IndexOf(adminRunTurnCmd) > -1) {
                                if (Settings.GameRunTurnByEmail) {
                                    Log("seems to be a Run Turn admin command, checking password...");
                                    string[] parts = body.Split('\n');
                                    int idx;
                                    for (int j = 0; j < parts.Length; j++) {
                                        if ((idx = parts[j].IndexOf(adminRunTurnCmd)) > -1) {
                                            string pw = parts[j].Substring(idx + adminRunTurnCmd.Length).Trim();
                                            if (pw.Equals(Settings.GameRunTurnByEmailPw))
                                                adminRunTurn = true;
                                            break;
                                        }
                                    }
                                } else {
                                    Logn("seems to be a Run Turn admin command, however this feature is disabled.  Ignoring.");
                                }
                            // Join command
                            } else if (body.IndexOf(joinCmd1) > -1 || body.IndexOf(joinCmd2) > -1) {
                                if (Settings.GameJoinByEmail) {
                                    Log("seems to be a join request, checking password and info...");
                                    bool joinGame = true;
                                    string[] parts;
                                    //string[] parts = body.Split('\n');
                                    //int idx;
                                    //bool joinGame = false;
                                    //for (int j = 0; j < parts.Length; j++) {
                                    //    if ((idx = parts[j].IndexOf(joinCmd)) > -1) {
                                    //        string pw = parts[j].Substring(idx + joinCmd.Length).Trim();
                                    //        if (pw.Equals(Settings.GameJoinByEmailPw))
                                    //            joinGame = true;
                                    //        break;
                                    //    }
                                    //}
                                    if (joinGame) {
                                        Dictionary<string, string> joininfo = new Dictionary<string, string>();
                                        joininfo.Add("Faction name:", "");
                                        joininfo.Add("Character name:", "");
                                        joininfo.Add("Starting city:", "");
                                        joininfo.Add("Your name:", "");
                                        joininfo.Add("Email address:", "");
                                        parts = body.Split('\n');
                                        for (int j = 0; j < parts.Length; j++) {
                                            foreach (string key in joininfo.Keys) {
                                                int w = parts[j].IndexOf(key);
                                                if (w != -1) {
                                                    joininfo[key] = parts[j].Substring(w + key.Length).Trim();
                                                    break;
                                                }
                                            }
                                        }
                                        bool ok = true;
                                        foreach (string key in joininfo.Keys)
                                            if (joininfo[key].Equals("")) {
                                                ok = false;
                                                break;
                                            }

                                        // Checking email address
                                        if (ok)
                                            ok = Olympia.IsValidEmail(joininfo["Email address:"]);

                                        // Checking for duplicate email address
                                        Boolean emailAlreadyExists = false;
                                        //  - in temp list
                                        if (ok) {
                                            ok = !IsEmailInTempList(joininfo["Email address:"]);
                                            emailAlreadyExists = true;
                                            if (ok)
                                                AddEmailToTempList(joininfo["Email address:"]);
                                        }
                                        //  - in player list
                                        if (ok) {
                                            ok = !IsEmailInGameList(joininfo["Email address:"]);
                                            emailAlreadyExists = true;
                                        }

                                        if (ok) {
                                            Log("ok, adding faction to faction queue...");
                                            AddFaction(joininfo["Faction name:"],
                                                joininfo["Character name:"],
                                                joininfo["Starting city:"],
                                                joininfo["Your name:"],
                                                joininfo["Email address:"]);
                                            string txt = "Hi," + Environment.NewLine + Environment.NewLine +
                                                "The registration of your faction was a success." + Environment.NewLine +
                                                "You will be added to the game when the next turn is run." + Environment.NewLine + Environment.NewLine +
                                                "The data you provided us was: " + Environment.NewLine +
                                                "Faction name: " + joininfo["Faction name:"] + Environment.NewLine +
                                                "Noble name: " + joininfo["Character name:"] + Environment.NewLine +
                                                "Starting city: " + joininfo["Starting city:"] + Environment.NewLine +
                                                "Player name: " + joininfo["Your name:"] + Environment.NewLine +
                                                "Player e-mail: " + joininfo["Email address:"] + Environment.NewLine + Environment.NewLine +
                                                "Your friendly Olympia administrator.";
                                            string subj = "Re: " + m.Subject;
                                            string to = "";
                                            if (m.ReplyToEmail != null)
                                                to = m.ReplyToEmail;
                                            else
                                                to = m.FromEmail;
                                            txt = "From: " + Settings.IncMailUser + Environment.NewLine +
                                                "Reply-To: " + Settings.IncMailUser + Environment.NewLine +
                                                "To: " + to + Environment.NewLine +
                                                "Subject: " + subj + Environment.NewLine + Environment.NewLine + txt;
                                            string fname = Olympia.UniqueFileName();
                                            StreamWriter sw = new StreamWriter(fname);
                                            sw.Write(txt);
                                            sw.Close();
                                            Logn("and sending confirmation e-mail:");
                                            Logn(" -- working, please wait --");
                                            Logn(RunProcess("sendmail", fname));
                                            Logn(" -- done --");
                                            File.Delete(fname);
                                        } else {
                                            if (emailAlreadyExists) {
                                                // Duplicate email
                                                Logn("Duplicate e-mail address, sending reply:");
                                                // send reply with error message
                                                string txt = "Hi," + Environment.NewLine + Environment.NewLine +
                                                    "You have attempted to join an Olympia game." + Environment.NewLine +
                                                    "However, there was something wrong with the information you provided." + Environment.NewLine +
                                                    "The e-mail addres you submitted is already in use by another active player." + Environment.NewLine + Environment.NewLine +
                                                    "Your original message was:" + Environment.NewLine + Environment.NewLine +
                                                    body;
                                                string subj = "Re: " + m.Subject;
                                                string to = "";
                                                if (m.ReplyToEmail != null)
                                                    to = m.ReplyToEmail;
                                                else
                                                    to = m.FromEmail;
                                                txt = "From: " + Settings.IncMailUser + Environment.NewLine +
                                                    "Reply-To: " + Settings.IncMailUser + Environment.NewLine +
                                                    "To: " + to + Environment.NewLine +
                                                    "Subject: " + subj + Environment.NewLine + Environment.NewLine + txt;
                                                string fname = Olympia.UniqueFileName();
                                                StreamWriter sw = new StreamWriter(fname);
                                                sw.Write(txt);
                                                sw.Close();
                                                Logn(" -- working, please wait --");
                                                Logn(RunProcess("sendmail", fname));
                                                Logn(" -- done --");
                                                File.Delete(fname);
                                            } else {
                                                if (Settings.GameJoinByEmailReplyWhenError) {
                                                    Logn("incomplete, sending reply:");
                                                    // send reply with error message
                                                    string txt = "Hi," + Environment.NewLine + Environment.NewLine +
                                                        "You have attempted to join an Olympia game." + Environment.NewLine +
                                                        "However, there was something wrong with the information you provided." + Environment.NewLine +
                                                        "Check out the following example:" + Environment.NewLine + Environment.NewLine +
                                                        "#join" + Environment.NewLine +
                                                        "Faction name:House of Atreides" + Environment.NewLine +
                                                        "Character name:Paul Atreides" + Environment.NewLine +
                                                        "Starting city:Harn" + Environment.NewLine +
                                                        "Your name:John Doe" + Environment.NewLine +
                                                        "Email address:john.doe@domain.com" + Environment.NewLine + Environment.NewLine +
                                                        "Your original message was:" + Environment.NewLine + Environment.NewLine +
                                                        body;
                                                    string subj = "Re: " + m.Subject;
                                                    string to = "";
                                                    if (m.ReplyToEmail != null)
                                                        to = m.ReplyToEmail;
                                                    else
                                                        to = m.FromEmail;
                                                    txt = "From: " + Settings.IncMailUser + Environment.NewLine +
                                                        "Reply-To: " + Settings.IncMailUser + Environment.NewLine +
                                                        "To: " + to + Environment.NewLine +
                                                        "Subject: " + subj + Environment.NewLine + Environment.NewLine + txt;
                                                    string fname = Olympia.UniqueFileName();
                                                    StreamWriter sw = new StreamWriter(fname);
                                                    sw.Write(txt);
                                                    sw.Close();
                                                    Logn(" -- working, please wait --");
                                                    Logn(RunProcess("sendmail", fname));
                                                    Logn(" -- done --");
                                                    File.Delete(fname);
                                                } else {
                                                    Logn("incomplete, ignoring.");
                                                }
                                            }
                                        }
                                    } else { 
                                    
                                    }
                                } else { 
                                    // Not allowed to join the game by e-mail
                                    Logn("seems to be a join request, however e-mail joining of this game is disallowed!");
                                }
                            // Orders
                            } else {
                                Log("might be orders, moving to order queue...");
                                // process webform lines and remove them
                                if (body.Contains("Submitted on")
                                    && body.Contains("Submitted by")
                                    && body.Contains("Submitted values are:")
                                    && body.Contains("Orders:")) {
                                    int idx = body.IndexOf("Orders:");
                                    if (idx > -1) {
                                        body = body.Substring(idx + 7);
                                    }
                                }
                                string[] lines = body.Split('\n');
                                StringBuilder sb = new StringBuilder("");
                                foreach (string line in lines)
                                    sb.Append(line.Trim() + "\n");
                                body = sb.ToString();
                                body = body.Replace("=A0", "");
                                string fname = Olympia.UniqueFileName();
                                StreamWriter sw = new StreamWriter(curdir + "\\lib\\spool\\m" + fname);
                                string rt = "";
                                if (m.ReplyToEmail != null && !m.ReplyToEmail.Trim().Equals(""))
                                    rt = m.ReplyToEmail;
                                else
                                    rt = m.FromEmail;
                                sw.Write("From " + m.FromEmail + Environment.NewLine +
                                    "Reply-To: " + rt + Environment.NewLine + 
                                    Environment.NewLine + 
                                    body);
                                sw.Close();
                                // Saving an extra copy to mailvault
                                if (!Directory.Exists(curdir + "\\mailvault"))
                                    Directory.CreateDirectory(curdir + "\\mailvault");
                                StreamWriter sw2 = new StreamWriter(curdir + "\\mailvault\\m" + fname);
                                sw2.Write("From " + m.FromEmail + Environment.NewLine +
                                    "Reply-To: " + rt + Environment.NewLine +
                                    Environment.NewLine +
                                    body);
                                sw2.Close();
                                Logn("done.");
                            }
                        }
                        if (!Settings.IncMailLeaveCopyOnServer)
                            popClient.DeleteMessage(i);
                    }
                    popClient.Disconnect();
                } catch (Exception ex) {
                    Logn("Error while attempting to retrieve gmail: " + ex.Message);
                    if (popClient.Connected)
                        popClient.Disconnect();
                }
            }
            string[] files = Directory.GetFiles(curdir + "\\lib\\spool");
            if (files.Length > 0)
                ScanOrders();
            // Check if we need to run the turn
            if (adminRunTurn) {
                StopTimer();
                RunTurn();
                timerStart = DateTime.Now;
                double mins = Convert.ToDouble(tbAutoEmailMins.Text);
                timerEnd = timerStart.AddMinutes(mins);
                StartTimer();
            }
            this.Cursor = Cursors.Default;
        }

        private void btnTestGmail_Click(object sender, EventArgs e) {
            CheckEmail();
        }

        private void ScanOrders() {
            this.Cursor = Cursors.WaitCursor;
            Log("Preparing order files...");
            string curdir = Directory.GetCurrentDirectory();
            // Add a "stop" file
            StreamWriter stopFile = new StreamWriter(curdir + "\\lib\\spool\\stop");
            stopFile.Close();
            Logn("done.");
            // Now scan the orders
            Logn("Scanning order files:");
            Logn(" -- working, please wait --");
            Log(RunProcess(curdir + "\\Olympia2.exe", "-w -e"));
            Logn(" -- done --");
            this.Cursor = Cursors.Default;
        }

        static public string UniqueFileName() { 
            DateTime now = DateTime.Now;
            string str = now.ToString("yyyyMMddHHmmssfff");
            while (File.Exists(str))
                str += random.Next(0, 10).ToString();
            return str;
        }

        static private bool IsValidEmail(string email) { 
            string uEmail = email.ToUpper();
            Regex r = new Regex("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}$");
            Match m = r.Match(uEmail);
            return m.Success;
        }

        private void btnAutoEmailStartStop_Click(object sender, EventArgs e) {
            Button btn = (Button)sender;
            if (btn.Text.Equals("Start")) {
                // check email settings
                if (!Settings.IncMailSettingsFileExists()) {
                    MessageBox.Show(this, "Incoming e-mail settings not found.  Please click \"E-mail Settings\" to configure your incoming e-mail account.");
                } else {
                    btn.Text = "...";
                    double mins = Convert.ToDouble(tbAutoEmailMins.Text);
                    if (mins >= 0) {
                        timerStart = DateTime.Now;
                        timerEnd = timerStart.AddMinutes(mins);
                        StartTimer();
                    } else {
                        MessageBox.Show(this, "Please fill in a correct number of minutes.");
                        btn.Text = "Start";
                    }
                }
            } else if (btn.Text.Equals("Stop")) {
                StopTimer();
            }
        }

        private void btnSimTurn_Click(object sender, EventArgs e) {
            string curdir = Directory.GetCurrentDirectory();
            Logn("Running turn:");
            Logn(" -- working, please wait --");
            Log(RunProcess(curdir + "\\Olympia2.exe", "-w -r"));
            Logn(" -- done --");
            UpdateCurrentTurn();
        }

        private void btnGenerateMap_Click(object sender, EventArgs e) {
            // plug in Sven's random map generator
            // random map generator will be located in \rndmap
            // it will output map files to \mapgen
            MessageBox.Show(this, "Not yet implemented.");
        }

        private void btnMapEditor_Click(object sender, EventArgs e) {
            string curdir = Directory.GetCurrentDirectory();
            string mapdir = curdir + "\\mapgen";
            // unzip mapgen folder if it doesn't exist
            if (!Directory.Exists(mapdir)) {
                Log("Installing map files...");
                cZip.UnZipFiles(curdir + "\\mapgen.zip", curdir, "", false);
                Logn("done.");
            }
            MapGUI mapgui = new MapGUI();
            mapgui.AutoLoadSaveMap();
            mapgui.ShowDialog();
        }

        private void btnGameSettings_Click(object sender, EventArgs e) {
            GameSettings f = new GameSettings();
            f.ShowDialog();
            Settings.LoadGameSettings();
            //Dictionary<string, string> gameSettings = GameSettings.LoadGameSettings();
            this.Text = Settings.GameName;
        }

        private void LogContextMenu_ItemClicked(object sender, ToolStripItemClickedEventArgs e) {
            if (e.ClickedItem.Equals(ClearLogMenuItem)) {
                tbOutput.Text = "";
            }
        }

        private void btnSubmitOrders_Click(object sender, EventArgs e) {
            string curdir = Directory.GetCurrentDirectory();

            SubmitOrders f = new SubmitOrders();
            f.ShowDialog();

            string[] files = Directory.GetFiles(curdir + "\\lib\\spool");
            if (files.Length > 0)
                ScanOrders();
        }

    }
}
