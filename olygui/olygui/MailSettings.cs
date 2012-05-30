using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace olygui {
    public partial class MailSettings : Form {

        bool saved = true;
        bool initializing;

        public MailSettings() {
            InitializeComponent();
        }

        private void btnOutMailReset_Click(object sender, EventArgs e) {
            // Outgoing mail settings
            tbOutMailHost.Text = "uit.telenet.be";
            cbOutMailSSL.Checked = false;
            tbOutMailPort.Text = "25";
            tbOutMailUser.Text = "";
            tbOutMailPw.Text = "";
            cbOutMailSendMail.Checked = true;
            saved = false;
        }

        private void SaveSettings() {
            // Outgoing Mail Settings
            Settings.OutMailHost = tbOutMailHost.Text.Trim();
            Settings.OutMailSSL = cbOutMailSSL.Checked;
            Settings.OutMailPort = Convert.ToUInt16(tbOutMailPort.Text.Trim());
            Settings.OutMailUser = tbOutMailUser.Text.Trim();
            Settings.OutMailPw = tbOutMailPw.Text;
            Settings.OutMailSendMail = cbOutMailSendMail.Checked;
            Settings.SaveOutMailSettings();

            // Incoming Mail Settings
            Settings.IncMailServer = tbIncMailServer.Text.Trim();
            Settings.IncMailSSL = cbIncMailSSL.Checked;
            Settings.IncMailPort = Convert.ToUInt16(tbIncMailPort.Text.Trim());
            Settings.IncMailUser = tbIncMailUser.Text.Trim();
            Settings.IncMailPw = tbIncMailPw.Text.Trim();
            Settings.IncMailLeaveCopyOnServer = cbIncMailLeaveCopyOnServer.Checked;
            Settings.SaveIncMailSettings();
            saved = true;
        }

        private void btnSaveAndClose_Click(object sender, EventArgs e) {
            if (!saved) {
                SaveSettings();
            }
            this.Close();
        }

        private void LoadSettings() {
            Settings.LoadIncMailSettings();
            tbOutMailHost.Text = Settings.OutMailHost;
            cbOutMailSSL.Checked = Settings.OutMailSSL;
            tbOutMailPort.Text = Settings.OutMailPort.ToString();
            tbOutMailUser.Text = Settings.OutMailUser;
            tbOutMailPw.Text = Settings.OutMailPw;
            cbOutMailSendMail.Checked = Settings.OutMailSendMail;
            Settings.LoadOutMailSettings();
            tbIncMailServer.Text = Settings.IncMailServer;
            cbIncMailSSL.Checked = Settings.IncMailSSL;
            tbIncMailPort.Text = Settings.IncMailPort.ToString();
            tbIncMailUser.Text = Settings.IncMailUser;
            tbIncMailPw.Text = Settings.IncMailPw;
            cbIncMailLeaveCopyOnServer.Checked = Settings.IncMailLeaveCopyOnServer;
        }

        private void MailSettings_Load(object sender, EventArgs e) {
            initializing = true;
            if (!Settings.IncMailSettingsFileExists())
                Settings.SaveIncMailSettings();
            if (!Settings.OutMailSettingsFileExists())
                Settings.SaveOutMailSettings();
            LoadSettings();
            initializing = false;
        }

        private void btnIncMailReset_Click(object sender, EventArgs e) {
            tbIncMailServer.Text = "pop.gmail.com";
            cbIncMailSSL.Checked = true;
            tbIncMailPort.Text = "995";
            tbIncMailUser.Text = "(Fill in your user name)";
            tbIncMailPw.Text = "(Fill in your password)";
            cbIncMailLeaveCopyOnServer.Checked = false;
            saved = false;
        }

        private void lblIncMailLeaveCopyOnServer_Click(object sender, EventArgs e) {
            cbIncMailLeaveCopyOnServer.Checked = !cbIncMailLeaveCopyOnServer.Checked;
        }

        private void lblIncMailSSL_Click(object sender, EventArgs e) {
            cbIncMailSSL.Checked = !cbIncMailSSL.Checked;
        }

        private void lblOutMailSSL_Click(object sender, EventArgs e) {
            cbOutMailSSL.Checked = !cbOutMailSSL.Checked;
        }

        private void lblOutMailSendMail_Click(object sender, EventArgs e) {
            cbOutMailSendMail.Checked = !cbOutMailSendMail.Checked;
        }

        private bool CheckSaved() {
            if (!saved) {
                DialogResult r = MessageBox.Show(this, "Do you wish to save these settings?", "Save settings", MessageBoxButtons.YesNoCancel);
                if (r == DialogResult.Yes) {
                    SaveSettings();
                    return true;
                } else if (r == DialogResult.No) {
                    saved = true;
                    return true;
                } else if (r == DialogResult.Cancel) {
                    return false;
                }
            }
            return true;
        }

        private void btnCancel_Click(object sender, EventArgs e) {
            if (CheckSaved())
                this.Close();
        }

        private void tbOutMailHost_TextChanged(object sender, EventArgs e) {
            if (!initializing)
                saved = false;
        }

        private void cbOutMailSSL_CheckedChanged(object sender, EventArgs e) {
            if (!initializing)
                saved = false;
        }

        private void tbOutMailPort_TextChanged(object sender, EventArgs e) {
            if (!initializing)
                saved = false;
        }

        private void tbOutMailUser_TextChanged(object sender, EventArgs e) {
            if (!initializing)
                saved = false;
        }

        private void tbOutMailPw_TextChanged(object sender, EventArgs e) {
            if (!initializing)
                saved = false;
        }

        private void cbOutMailSendMail_CheckedChanged(object sender, EventArgs e) {
            if (!initializing)
                saved = false;
        }

        private void tbIncMailServer_TextChanged(object sender, EventArgs e) {
            if (!initializing)
                saved = false;
        }

        private void cbIncMailSSL_CheckedChanged(object sender, EventArgs e) {
            if (!initializing)
                saved = false;
        }

        private void tbIncMailPort_TextChanged(object sender, EventArgs e) {
            if (!initializing)
                saved = false;
        }

        private void tbIncMailUser_TextChanged(object sender, EventArgs e) {
            if (!initializing)
                saved = false;
        }

        private void cbIncMailLeaveCopyOnServer_CheckedChanged(object sender, EventArgs e) {
            if (!initializing)
                saved = false;
        }

        private void tbIncMailPw_TextChanged(object sender, EventArgs e) {
            if (!initializing)
                saved = false;
        }

        private void MailSettings_FormClosing(object sender, FormClosingEventArgs e) {
            if (!CheckSaved())
                e.Cancel = true;
        }
    }
}
