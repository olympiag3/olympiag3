using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace olygui {

    public partial class GameSettings : Form {

        private static string setFile = Directory.GetCurrentDirectory() + "\\game.ini";

        public GameSettings() {
            InitializeComponent();
            Settings.LoadGameSettings();
            tbGameName.Text = Settings.GameName;
            cbJoinByEmail.Checked = Settings.GameJoinByEmail;
            tbJoinByEmailPw.Text = Settings.GameJoinByEmailPw;
            cbJoinGameByEmailReplyWhenError.Checked = Settings.GameJoinByEmailReplyWhenError;
            tbHtmlReportsFolder.Text = Settings.GameHtmlReportsFolder;
            tbPublicHtmlReportsFolder.Text = Settings.GamePublicHtmlReportsFolder;
            tbHtmlTimesFolder.Text = Settings.GameHtmlTimesFolder;
            cbRunTurnByEmail.Checked = Settings.GameRunTurnByEmail;
            tbRunTurnByEmailPw.Text = Settings.GameRunTurnByEmailPw;
        }

        private void btnSaveAndClose_Click(object sender, EventArgs e) {
            if (cbRunTurnByEmail.Checked && tbRunTurnByEmailPw.Text.Trim().Equals("")) {
                MessageBox.Show(this, "Run Turn by E-mail is enabled, but a password has not been set.  Cannot save these settings until this issue is resolved!");
                return;
            }
            Settings.GameName = tbGameName.Text.Trim();
            Settings.GameJoinByEmail = cbJoinByEmail.Checked;
            Settings.GameJoinByEmailPw = tbJoinByEmailPw.Text;
            Settings.GameJoinByEmailReplyWhenError = cbJoinGameByEmailReplyWhenError.Checked;
            Settings.GameHtmlReportsFolder = tbHtmlReportsFolder.Text;
            Settings.GamePublicHtmlReportsFolder = tbPublicHtmlReportsFolder.Text;
            Settings.GameHtmlTimesFolder = tbHtmlTimesFolder.Text;
            Settings.GameRunTurnByEmail = cbRunTurnByEmail.Checked;
            Settings.GameRunTurnByEmailPw = tbRunTurnByEmailPw.Text.Trim();
            Settings.SaveGameSettings();
            this.Close();
        }

        private void lblJoinByEmail_Click(object sender, EventArgs e) {
            cbJoinByEmail.Checked = !cbJoinByEmail.Checked;
        }

        private void btnHtmlReportsFolderBrowse_Click(object sender, EventArgs e) {
            string path = tbHtmlReportsFolder.Text;
            if (!Directory.Exists(path))
                path = Directory.GetCurrentDirectory() + "\\lib\\html";
            FolderBrowserDialog f = new FolderBrowserDialog();
            f.SelectedPath = path;
            if (f.ShowDialog() == DialogResult.OK) {
                tbHtmlReportsFolder.Text = f.SelectedPath;
            }
        }

        private void btnHtmlTimesFolderBrowse_Click(object sender, EventArgs e) {
            string path = tbHtmlTimesFolder.Text;
            if (!Directory.Exists(path))
                path = Directory.GetCurrentDirectory() + "\\times";
            FolderBrowserDialog f = new FolderBrowserDialog();
            f.SelectedPath = path;
            if (f.ShowDialog() == DialogResult.OK) {
                tbHtmlTimesFolder.Text = f.SelectedPath;
            }
        }

        private void btnPublicHtmlReportsFolderBrowse_Click(object sender, EventArgs e) {
            string path = tbPublicHtmlReportsFolder.Text;
            if (!Directory.Exists(path))
                path = Directory.GetCurrentDirectory() + "\\public";
            FolderBrowserDialog f = new FolderBrowserDialog();
            f.SelectedPath = path;
            if (f.ShowDialog() == DialogResult.OK) {
                tbPublicHtmlReportsFolder.Text = f.SelectedPath;
            }
        }


    }

}
