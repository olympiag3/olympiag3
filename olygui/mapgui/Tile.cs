using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;

namespace mapgui {

    public class Tile : IComparable<Tile> {
        private Coord xy;
        private char c;
        private List<MapTag> tags;
        static Dictionary<char, Color> colors = new Dictionary<char,Color>();
        static bool colorsLoaded = false;

        public Tile() {
            xy = new Coord(0, 0);
            c = '\0';
            tags = new List<MapTag>();
            if (!colorsLoaded) {
                LoadColors();
                colorsLoaded = true;
            }
        }

        public Tile(int x, int y, char c) : this() {
            xy = new Coord(x, y);
            this.c = c;
        }

        public int X { get { return xy.x; } set { xy.x = value; } }
        public int Y { get { return xy.y; } set { xy.y = value; } }
        public char C { get { return c; } set { c = value; } }
        public Color Color { 
            get { 
                if (colors.ContainsKey(c))
                    return colors[c];
                return Color.Blue;
            } 
        }
        public void AddTag(MapTag tag) {
            tags.Add(tag);
        }
        public List<MapTag> Tags { get { return tags; } }

        public bool IsLand() {
            string water = ";:~\",. '";
            return (water.IndexOf(c) == -1);
        }

        public int CompareTo(Tile t) {
            int i1 = Y * 100000 + X;
            int i2 = t.Y * 100000 + t.X;
            return i1.CompareTo(i2);
        }

        static public Color ColorForChar(char c) {
            if (!colorsLoaded) {
                LoadColors();
                colorsLoaded = true;
            }
            if (colors.ContainsKey(c))
                return colors[c];
            return Color.White;
        }

        static private void LoadColors() {
            // Sea lane
            colors.Add(';', Color.LightBlue);
            colors.Add(':', Color.LightBlue);
            colors.Add('~', Color.LightBlue);
            colors.Add('"', Color.LightBlue);
            // Ocean
            colors.Add(',', Color.Blue);
            colors.Add('.', Color.Blue);
            colors.Add(' ', Color.Blue);
            colors.Add('\'', Color.Blue);
            // Plain
            colors.Add('p', Color.YellowGreen);
            colors.Add('P', Color.YellowGreen);
            // Desert
            colors.Add('d', Color.Gold);
            colors.Add('D', Color.Gold);
            // Mountain
            colors.Add('m', Color.Red);
            colors.Add('M', Color.Red);
            colors.Add('^', Color.Red);
            colors.Add('v', Color.Red);
            colors.Add('{', Color.Red);
            colors.Add('}', Color.Red);
            colors.Add('0', Color.Red);
            // Swamp
            colors.Add('s', Color.Orange);
            colors.Add('S', Color.Orange);
            colors.Add('[', Color.Orange);
            colors.Add(']', Color.Orange);
            // Forest
            colors.Add('f', Color.Green);
            colors.Add('F', Color.Green);
            colors.Add('1', Color.Green);
            colors.Add('2', Color.Green);
            colors.Add('3', Color.Green);
            colors.Add('4', Color.Green);
            colors.Add('5', Color.Green);
            colors.Add('6', Color.Green);
            colors.Add('7', Color.Green);
            colors.Add('8', Color.Green);
            // Random
            colors.Add('o', Color.LightGray);
            // Land
            colors.Add('*', Color.Brown);
            colors.Add('%', Color.Brown);
        }

    }

}
