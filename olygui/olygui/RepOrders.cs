using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace olygui {
    public partial class RepOrders : Form {
        public RepOrders() {
            InitializeComponent();
        }

        string currentFile = "";

        private void save() {
            string curdir = Directory.GetCurrentDirectory();
            // If we were already looking at a file, save it
            if (!currentFile.Equals("")) {
                StreamWriter sw = new StreamWriter(curdir + "\\lib\\spool\\" + currentFile);
                sw.Write(tbOrders.Text);
                sw.Close();
            }
        }

        private void RepOrders_Load(object sender, EventArgs e) {
            cbTurnReport.Items.Clear();
            cbTurnReport.Items.Add("");
            // Load file list from rep folder
            string curdir = Directory.GetCurrentDirectory();
            string[] files = Directory.GetFiles(curdir + "\\rep");
            foreach (string file in files) {
                string[] parts = file.Split('\\');
                cbTurnReport.Items.Add(parts[parts.Length-1]);
            }
        }

        private void RepOrders_FormClosing(object sender, FormClosingEventArgs e) {
            save();
        }

        private void cbTurnReport_SelectedIndexChanged(object sender, EventArgs e) {
            string curdir = Directory.GetCurrentDirectory();
            string file = (string)cbTurnReport.SelectedItem;
            if (File.Exists(curdir + "\\rep\\" + file)) {
                save();
                // and go on
                currentFile = file;
                StreamReader sr = new StreamReader(curdir + "\\rep\\" + currentFile);
                tbReport.Text = sr.ReadToEnd();
                sr.Close();
                // check if we already have an order file for this one
                if (File.Exists(curdir + "\\lib\\spool\\" + currentFile)) {
                    // we do, open it
                    sr = new StreamReader(curdir + "\\lib\\spool\\" + currentFile);
                    tbOrders.Text = sr.ReadToEnd();
                    sr.Close();
                } else { 
                    // we don't, copy the order template from the report
                    string rep = tbReport.Text;
                    int start = rep.IndexOf("Order template");
                    if (start > -1) {
                        start = rep.IndexOf("begin", start);
                        tbOrders.Text = rep.Substring(start);
                    } else {
                        // the turn report is broken, so nothing to do but provide an empty order field
                        tbOrders.Text = "";
                    }
                }
            } else {
                MessageBox.Show("An error occurred.  Report file does not exist.");
            }
        }
    }
}
