using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace olygui {
    public partial class SubmitOrders : Form {

        public SubmitOrders() {
            InitializeComponent();
        }

        private void btnOk_Click(object sender, EventArgs e) {
            string curdir = Directory.GetCurrentDirectory();
            string fname = Olympia.UniqueFileName();
            StreamWriter sw = new StreamWriter(curdir + "\\lib\\spool\\m" + fname);
            sw.Write("From john@doe.com" + Environment.NewLine +
                "Reply-To: john@doe.com" + Environment.NewLine +
                Environment.NewLine +
                tbOrders.Text);
            sw.Close();
            this.Close();
        }
    }
}
