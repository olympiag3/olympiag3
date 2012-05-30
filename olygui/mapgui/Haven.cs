using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace mapgui {

    public class Haven {

        private char c;
        private string name;

        public char C { get { return c; } set { c = value; } }
        public string Name { get { return name; } set { name = value; } }

        public Haven() {
            c = '\0';
            name = "";
        }

        public Haven(char c, string name) : this() {
            this.c = c;
            this.name = name;
        }

        public override string ToString() {
            return String.Format("{0:c}: {1:s}", c, name);
        }


    }

}
