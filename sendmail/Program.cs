using System;
using System.Text;
using System.Net;
using System.Net.Mail;
using System.IO;
using System.Collections.Generic;

namespace sendmail {

    class Program {

        static private void Log(Exception ex) {
            try {
                Log(ex.ToString());
            } catch {
                throw new Exception("Log could not be written!");
            }
        }

        static private void Log(string txt) {
            try {
                StreamWriter log = new StreamWriter("sendmail.log", true);
                string d = DateTime.Now.ToString();
                log.WriteLine(d + "\t" + txt);
                log.Close();
                Console.WriteLine(txt);
            } catch {
                throw new Exception("Log could not be written!");
            }
        }

        static void Main(string[] args) {
            try {
                if (args.Length > 0) {
                    String emailFile = args[0];
                    StringBuilder report = new StringBuilder();

                    String from = "";
                    String to = "";
                    String replyTo = "";
                    String subject = "";
                    String body = "";
                    string bcc = "";
                    string xloop = "";

                    if (File.Exists(emailFile)) {
                        // First load settings
                        Settings.LoadGameSettings();
                        Settings.LoadIncMailSettings();
                        Settings.LoadOutMailSettings();
                        // Read the file we need to send
                        StreamReader sr = new StreamReader(emailFile);
                        string[] fnameparts = emailFile.Split('\\');
                        if (fnameparts.Length == 1)
                            fnameparts = emailFile.Split('/');
                        string dirname = Directory.GetCurrentDirectory() + "\\mailvault";
                        string d = DateTime.Now.ToString("yyyyMMdd_HHmmssfff_");
                        if (!Directory.Exists(dirname))
                            Directory.CreateDirectory(dirname);
                        StreamWriter sw = new StreamWriter(dirname + "\\" + d + fnameparts[fnameparts.Length - 1]);
                        try {
                            // first read the message stuff
                            bool done = false;
                            bool firstline = true;
                            while (!sr.EndOfStream) {
                                string line = sr.ReadLine();
                                sw.WriteLine(line);
                                if (firstline && line.Trim().Equals(""))
                                    continue;
                                else
                                    firstline = false;
                                if (!done) {
                                    if (line.ToLower().Contains("to:")) {
                                        to = line.Split(':')[1].Trim();
                                    } else if (line.ToLower().Contains("subject:")) {
                                        String[] parts = line.Split(':');
                                        subject = String.Join(":", parts, 1, parts.Length - 1);
                                    } else if (line.ToLower().Contains("bcc:")) {
                                        bcc = line.Split(' ')[1];
                                    } else if (line.ToLower().Contains("x-loop:")) {
                                        xloop = line.Split(' ')[1];
                                    } else if (line.Trim().Equals("")) {
                                        done = true;
                                    }
                                } else {
                                    report.AppendLine(line);
                                }
                            }
                        } catch (Exception ex) {
                            throw ex;
                        } finally {
                            sw.Close();
                            sr.Close();
                        }
                        body = report.ToString();

                        // Replace From and Reply-to with incoming mail user
                        from = Settings.IncMailUser;
                        replyTo = Settings.IncMailUser;
                        // Add game name to subject
                        subject = Settings.GameName + " - " + subject;

                        MailMessage msg = new MailMessage();
                        msg.From = new MailAddress(from);
                        msg.To.Add(to);
                        msg.Subject = subject;
                        
                        AlternateView av = AlternateView.CreateAlternateViewFromString(body, Encoding.UTF8, "text/plain");
                        av.TransferEncoding = System.Net.Mime.TransferEncoding.SevenBit;
                        msg.AlternateViews.Add(av);

                        MailAddress replyToAddress = new MailAddress(replyTo);
                        msg.ReplyTo = replyToAddress;
                        if (!bcc.Equals(""))
                            msg.Bcc.Add(new MailAddress(bcc));

                        if (Settings.OutMailSendMail) {
                            SmtpClient smtpClient = new SmtpClient();
                            smtpClient.Host = Settings.OutMailHost;
                            smtpClient.EnableSsl = Settings.OutMailSSL;
                            smtpClient.Port = (int)Settings.OutMailPort;
                            if (!Settings.OutMailUser.Equals("") || !Settings.OutMailPw.Equals("")) {
                                smtpClient.Credentials = new NetworkCredential(Settings.OutMailUser, Settings.OutMailPw);
                                smtpClient.UseDefaultCredentials = true;
                            }
                            smtpClient.Send(msg);
                            Log("E-mail sent from " + from + " (reply-to: " + ") to " + to + " with subject: " + subject);
                        }
                    } else {
                        Log("File " + emailFile + " does not exist.");
                    }
                }
            } catch (SmtpException ex) {
                Log(ex);
            } catch (Exception ex) {
                Log(ex);
            }
        }
    }
}
