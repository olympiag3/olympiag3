using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace mapgui {

    public class Region {

        private int x;
        private int y;
        private string name;

        public int X { get { return x; } set { x = value; } }
        public int Y { get { return y; } set { y = value; } }
        public string Name { get { return name; } set { name = value; } }

        public Region() {
            x = 0;
            y = 0;
            name = "";
        }

        public Region(int x, int y, string name) : this() {
            this.x = x;
            this.y = y;
            this.name = name;
        }

        public override string ToString() {
            return String.Format("{0:d},{1:d}: {2:s}", x, y, name);
        }

    }

}
