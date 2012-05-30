using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace mapgui {

    public class Land {

        private int x;
        private int y;
        private char c;
        private string name;

        public int X { get { return x; } set { x = value; } }
        public int Y { get { return y; } set { y = value; } }
        public char C { get { return c; } set { c = value; } }
        public string Name { get { return name; } set { name = value; } }

        public Land() {
            x = 0;
            y = 0;
            c = ' ';
            name = "";
        }

        public Land(int x, int y, char c, string name) : this() {
            this.x = x;
            this.y = y;
            this.c = c;
            this.name = name;
        }

        public override string ToString() {
            return String.Format("{0:d},{1:d} ({2:c}): {3:s}", x, y, c, name);
        }

        public override bool Equals(object obj) {
            if (obj == null || GetType() != obj.GetType())
                return false;
            Land o = (Land)obj;
            return (o.x.Equals(x) 
                && o.y.Equals(y)
                && o.c.Equals(c)
                && o.name.Equals(name));
        }

    }

}
