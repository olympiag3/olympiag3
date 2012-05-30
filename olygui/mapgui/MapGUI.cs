using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace mapgui {

    enum State { 
        Idle,
        MovingLand,
        PlacingLand,
        Editing,
        MovingRegion,
        PlacingRegion
    }

    public partial class MapGUI : Form {

        // Files
        string mapFile;

        // Data
        Dictionary<Coord, Tile> tiles;
        List<string> cities;
        List<Land> lands;
        List<Region> regions;
        Dictionary<char, Haven> havens;

        bool autoLoadSaveMap = false;

        // Map GUI
        float bsize = 10;
        bool mouseOverMap = false;
        int mapWidth = 0;
        int mapHeight = 0;
        float mapOffsetX = 0;
        float mapOffsetY = 0;
        float dragX = 0;
        float dragY = 0;
        DateTime dragStartTime = DateTime.Now;
        bool grid = true;
        bool dragging = false;

        State state;
        int lastSelectedLand = -1;
        int lastSelectedRegion = -1;
        int lastSelectedHaven = -1;

        // Edit
        int boxSelected = 0;

        // Internal
        Bitmap BackBuffer;

        int drawx = -1;
        int drawy = -1;
        int draww = -1;
        int drawh = -1;

        public MapGUI() {
            InitializeComponent();
            state = State.Idle;
            mapFile = "";
            tiles = new Dictionary<Coord, Tile>();
            cities = new List<string>();
            lands = new List<Land>();
            regions = new List<Region>();
            havens = new Dictionary<char, Haven>(); 
            BackBuffer = new Bitmap(this.ClientSize.Width, this.ClientSize.Height);
            map.Paint += new PaintEventHandler(map_Paint);
            this.MouseWheel += new MouseEventHandler(map_MouseWheel);
            map.MouseEnter += new EventHandler(map_MouseEnter);
            map.MouseMove += new MouseEventHandler(map_MouseMove);
            map.MouseLeave += new EventHandler(map_MouseLeave);
            map.MouseDown += new MouseEventHandler(map_MouseDown);
            map.MouseUp += new MouseEventHandler(map_MouseUp); 
        }
         
        public void AutoLoadSaveMap() {
            autoLoadSaveMap = true;
            mapFile = Directory.GetCurrentDirectory() + "\\mapgen\\Map";
        }


        private void LoadCities() {
            if (!mapFile.Equals("")) {
                string path = Directory.GetParent(mapFile).ToString();
                string cfile = path + "\\Cities";
                if (File.Exists(cfile)) {
                    cities.Clear();
                    StreamReader sr = new StreamReader(cfile);
                    while (!sr.EndOfStream)
                        cities.Add(sr.ReadLine().Trim());
                    sr.Close();
                    lbCities.Items.Clear();
                    lbCities.Items.AddRange(cities.ToArray());
                }
            }
        }

        private void SaveHavens() {
            if (!mapFile.Equals("")) {
                string path = Directory.GetParent(mapFile).ToString();
                string hfile = path + "\\Havens";
                StreamWriter sw = new StreamWriter(hfile, false);
                foreach (Haven h in havens.Values)
                    sw.WriteLine(String.Format("{0:c} {1:s}", h.C, h.Name));
                sw.Close();
            }
        }

        private void LoadHavens() {
            if (!mapFile.Equals("")) {
                string path = Directory.GetParent(mapFile).ToString();
                string hfile = path + "\\Havens";
                if (File.Exists(hfile)) {
                    havens.Clear();
                    StreamReader sr = new StreamReader(hfile);
                    int i = 1;
                    while (!sr.EndOfStream) {
                        string line = sr.ReadLine();
                        Haven h = new Haven(Convert.ToChar(line.Substring(0,1)), line.Substring(2));
                        havens.Add(h.C, h);
                        if (tlpEditHavens.Controls.ContainsKey("pnlHaven" + i.ToString()) && tlpEditHavens.Controls.ContainsKey("lblHaven" + i.ToString())) {
                            Panel pnl = (Panel)tlpEditHavens.Controls["pnlHaven" + i.ToString()];
                            pnl.BackColor = Tile.ColorForChar(h.C);
                            Label lbl = (Label)tlpEditHavens.Controls["lblHaven" + i.ToString()];
                            lbl.Text = h.Name;
                        }
                        i++;
                    }
                    sr.Close();
                    lbHavens.Items.Clear();
                    foreach(Haven h in havens.Values)
                        lbHavens.Items.Add(h);
                }
            }
        }

        private void SaveRegions() {
            if (!mapFile.Equals("")) {
                string path = Directory.GetParent(mapFile).ToString();
                string lfile = path + "\\Regions";
                StreamWriter sw = new StreamWriter(lfile, false);
                foreach (Region r in regions)
                    sw.WriteLine(String.Format("{0:d},{1:d}\t{2:s}", r.Y, r.X, r.Name));
                sw.Close();
            }
        }

        private void LoadRegions() {
            if (!mapFile.Equals("")) {
                string path = Directory.GetParent(mapFile).ToString();
                string lfile = path + "\\Regions";
                if (File.Exists(lfile)) {
                    regions.Clear();
                    StreamReader sr = new StreamReader(lfile);
                    while (!sr.EndOfStream) {
                        string line = sr.ReadLine();
                        int x = 0;
                        int y = 0;
                        string name = "";
                        int idx = line.IndexOf(',');
                        if (idx > -1) {
                            y = Convert.ToInt32(line.Substring(0, idx));
                            line = line.Substring(idx + 1);
                        }
                        idx = line.IndexOf('\t');
                        if (idx > -1) {
                            x = Convert.ToInt32(line.Substring(0, idx));
                            line = line.Substring(idx + 1);
                        }
                        name = line.Trim();
                        Region r = new Region(x, y, name);
                        regions.Add(r);
                    }
                    sr.Close();
                    lbRegions.Items.Clear();
                    lbRegions.Items.AddRange(regions.ToArray());
                }
            }
        }

        private void SaveLands() {
            if (!mapFile.Equals("")) {
                string path = Directory.GetParent(mapFile).ToString();
                string lfile = path + "\\Land";
                StreamWriter sw = new StreamWriter(lfile, false);
                foreach (Land l in lands)
                    sw.WriteLine(String.Format("{0:d},{1:d}\t{2:c}\t{3:s}", l.Y, l.X, l.C, l.Name));
                sw.Close();
            }
        }

        private void LoadLands() {
            if (!mapFile.Equals("")) {
                string path = Directory.GetParent(mapFile).ToString();
                string lfile = path + "\\Land";
                if (File.Exists(lfile)) {
                    lands.Clear();
                    StreamReader sr = new StreamReader(lfile);
                    while (!sr.EndOfStream) {
                        string line = sr.ReadLine();
                        int x = 0;
                        int y = 0;
                        char c = '\0';
                        string name = "";
                        int idx = line.IndexOf(',');
                        if (idx > -1) {
                            y = Convert.ToInt32(line.Substring(0, idx));
                            line = line.Substring(idx + 1);
                        }
                        idx = line.IndexOf('\t');
                        if (idx > -1) {
                            x = Convert.ToInt32(line.Substring(0, idx));
                            line = line.Substring(idx + 1);
                        }
                        c = Convert.ToChar(line.Substring(0, 1));
                        name = line.Substring(2).Trim();
                        Land l = new Land(x, y, c, name);
                        lands.Add(l);
                    }
                    sr.Close();
                    lbLands.Items.Clear();
                    lbLands.Items.AddRange(lands.ToArray());
                }
            }
        }

        private void LoadMapFile() {
            mapWidth = 0;
            mapHeight = 0;
            int x = 0;
            int y = 0;
            tiles = new Dictionary<Coord, Tile>();
            if (mapFile == "")
                mapFile = Directory.GetCurrentDirectory() + "\\Map";
            if (File.Exists(mapFile)) {
                StreamReader sr = new StreamReader(mapFile);
                while (!sr.EndOfStream) {
                    char[] chars = sr.ReadLine().ToCharArray();
                    if (mapWidth == 0) {
                        mapWidth = chars.Length;
                    }
                    x = 0;
                    foreach (char c in chars)
                        tiles.Add(new Coord(x,y), new Tile(x++, y, c));
                    y++;
                }
                sr.Close();
                mapHeight = y;
            }
        }

        private void MapGUI_Load(object sender, EventArgs e) {
            pnlForest.BackColor = Tile.ColorForChar('f');
            pnlPlain.BackColor = Tile.ColorForChar('p');
            pnlDesert.BackColor = Tile.ColorForChar('d');
            pnlMountain.BackColor = Tile.ColorForChar('m');
            pnlSwamp.BackColor = Tile.ColorForChar('s');
            pnlRandom.BackColor = Tile.ColorForChar('o');
            pnlOcean.BackColor = Tile.ColorForChar('.');
            pnlSealane.BackColor = Tile.ColorForChar(':');
            pnlPortCity.BackColor = Tile.ColorForChar('%');
            if (autoLoadSaveMap) {
                btnLoadMap.Visible = false;
                btnSaveMap.Visible = false;
                LoadMap();
            }
        }

        private void chkShowGrid_CheckedChanged(object sender, EventArgs e) {
            CheckBox cb = ((CheckBox)sender);
            if (cb.Checked)
                grid = true;
            else
                grid = false;
            map.Refresh();
        }

        private void map_Resize(object sender, EventArgs e) {
            if (this.ClientSize.Width > 0 && this.ClientSize.Height > 0) {
                BackBuffer = new Bitmap(this.ClientSize.Width, this.ClientSize.Height);
                map.Refresh();
            }
        }

        private void LoadMap() {
            LoadMapFile();
            LoadLands();
            LoadRegions();
            LoadCities();
            LoadHavens();
            TagLands();
            TagRegions();
            TagHavens();
            map.Refresh();
            state = State.Idle;
        }

        private void btnLoadMap_Click(object sender, EventArgs e) {
            string curdir = Directory.GetCurrentDirectory();
            OpenFileDialog fd = new OpenFileDialog();
            fd.InitialDirectory = curdir;
            if (fd.ShowDialog() == DialogResult.OK) {
                mapFile = fd.FileName;
            }
            LoadMap();
        }

        private void SaveMap() {
            if (CheckMapConsistency() && !mapFile.Equals("")) {
                string curdir = Directory.GetCurrentDirectory();
                // Save Map
                StreamWriter sw = new StreamWriter(mapFile);
                List<Tile> sortedTiles = new List<Tile>(tiles.Values);
                sortedTiles.Sort();
                StringBuilder line = new StringBuilder("");
                int y = -1;
                foreach (Tile t in sortedTiles) {
                    if (y == -1)
                        y = t.Y;
                    if (t.Y == y)
                        line.Append(t.C);
                    else {
                        sw.WriteLine(line);
                        y = t.Y;
                        line = new StringBuilder("");
                        line.Append(t.C);
                    }
                }
                sw.WriteLine(line);
                sw.Close();
                // Save Lands
                SaveLands();
                // Save Regions
                SaveRegions();
                // Save Havens
                SaveHavens();
            }
        }

        private void btnSaveMap_Click(object sender, EventArgs e) {
            SaveMap();
        }

        private void TagLands() {
            foreach (Land l in lands) {
                Coord xy = new Coord(l.X, l.Y);
                if (tiles.ContainsKey(xy))
                    tiles[xy].AddTag(new MapTag(l.Name, TagType.Land));
            }
        }

        private void TagRegions() {
            foreach (Region r in regions) {
                Coord xy = new Coord(r.X, r.Y);
                if (tiles.ContainsKey(xy))
                    tiles[xy].AddTag(new MapTag(r.Name, TagType.Region));
            }
        }

        private void TagHavens() {
            foreach (Tile t in tiles.Values)
                if (havens.ContainsKey(t.C))
                    t.AddTag(new MapTag(havens[t.C].Name, TagType.Haven));
        }

        private bool ShowTagType(TagType type) {
            bool retval;
            switch (type) { 
                case TagType.Land:
                    retval = cbFilterLands.Checked;
                break;
                case TagType.Region:
                    retval = cbFilterRegions.Checked;
                break;
                case TagType.Haven:
                    retval = cbFilterHavens.Checked;
                break;
                default:
                retval = false;
                break;
            }
            return retval;
        }

        private void PlaceLand(int x, int y) {
            Land l = (Land)lbLands.SelectedItem;
            MapTag tag = new MapTag(l.Name, TagType.Land);
            // Add new tag to tiles
            Coord xy = new Coord(x, y);
            tiles[xy].Tags.Add(tag);
            foreach (Land land in lands) {
                if (land.Equals(l)) {
                    land.X = x;
                    land.Y = y;
                    land.C = tiles[xy].C;
                    break;
                }
            }
            UpdateLandList();
            map.Refresh();
            state = State.MovingLand;
        }

        private void UpdateLandList() {
            int selected = lbLands.SelectedIndex;
            lbLands.Items.Clear();
            lbLands.Items.AddRange(lands.ToArray());
            lbLands.SelectedIndex = selected;
        }

        private void UpdateRegionList() {
            int selected = lbRegions.SelectedIndex;
            lbRegions.Items.Clear();
            lbRegions.Items.AddRange(regions.ToArray());
            lbRegions.SelectedIndex = selected;
        }

        private void MoveLand(int x, int y) {
            Land l = (Land)lbLands.SelectedItem;
            MapTag tag = new MapTag(l.Name, TagType.Land);
            // Remove old tag from tiles
            Coord xy = new Coord(l.X, l.Y);
            foreach (MapTag t in tiles[xy].Tags) {
                if (t.Equals(tag)) {
                    tiles[xy].Tags.Remove(t);
                    break;
                }
            }
            // Add new tag to tiles
            xy = new Coord(x, y);
            tiles[xy].Tags.Add(tag);
            foreach (Land land in lands) { 
                if (land.Equals(l)) {
                    land.X = x;
                    land.Y = y;
                    land.C = tiles[xy].C;
                    break;
                }
            }
            UpdateLandList();
            map.Refresh();
        }

        private void PlaceRegion(int x, int y) {
            Region l = (Region)lbRegions.SelectedItem;
            MapTag tag = new MapTag(l.Name, TagType.Region);
            // Add new tag to tiles
            Coord xy = new Coord(x, y);
            tiles[xy].Tags.Add(tag);
            l.X = x;
            l.Y = y;
            UpdateRegionList();
            map.Refresh();
            state = State.MovingRegion;
        }

        private void MoveRegion(int x, int y) {
            Region l = (Region)lbRegions.SelectedItem;
            MapTag tag = new MapTag(l.Name, TagType.Region);
            // Remove old tag from tiles
            Coord xy = new Coord(l.X, l.Y);
            foreach (MapTag t in tiles[xy].Tags) {
                if (t.Equals(tag)) {
                    tiles[xy].Tags.Remove(t);
                    break;
                }
            }
            // Add new tag to tiles
            xy = new Coord(x, y);
            tiles[xy].Tags.Add(tag);
            l.X = x;
            l.Y = y;
            UpdateRegionList();
            map.Refresh();
        }
        
        private void DrawOnMap(int x, int y) {
            if (state == State.Editing && boxSelected != 0) {
                Coord xy = new Coord(x, y);
                if (tiles.ContainsKey(xy)) {
                    Tile t = tiles[xy];
                    switch (boxSelected) {
                        case 1:
                            // Forest
                            t.C = 'f';
                            break;
                        case 2:
                            // Plain
                            t.C = 'p';
                            break;
                        case 3:
                            // Mountain
                            t.C = 'm';
                            break;
                        case 4:
                            // Desert
                            t.C = 'd';
                            break;
                        case 5:
                            // Swamp
                            t.C = 's';
                            break;
                        case 6:
                            // Random
                            t.C = 'o';
                            break;
                        case 7:
                            // Ocean
                            t.C = '.';
                            break;
                        case 8:
                            // Sealane
                            t.C = ':';
                            break;
                        case 9:
                            // Port City
                            t.C = '%';
                            break;
                    }
                    map.Refresh();
                }
            }
        }

        private void cbFilterLands_CheckedChanged(object sender, EventArgs e) {
            map.Refresh();
        }

        private void cbFilterRegions_CheckedChanged(object sender, EventArgs e) {
            map.Refresh();
        }

        private void cbFilterHavens_CheckedChanged(object sender, EventArgs e) {
            map.Refresh();
        }

        private void map_Paint(object sender, PaintEventArgs e) {
            int ld = 25;
            if (tiles.Count > 0) {
                Graphics g = Graphics.FromImage(BackBuffer);
                Pen p = new Pen(Color.DarkGray);
                SolidBrush wb = new SolidBrush(Color.White);
                int boxes = 0;
                // Tiles
                foreach (Tile t in tiles.Values) {
                    float x = mapOffsetX + (bsize * (float)t.X);
                    float y = mapOffsetY + (bsize * (float)t.Y);
                    float w = x + bsize;
                    float h = y + bsize;
                    // only draw what's in the viewport (0, 0, width, height)
//                    if (w > 0 && h > 0 && x < e.ClipRectangle.Width && y < e.ClipRectangle.Height) {
                    if (((drawx != -1 && drawy != -1 && draww != -1 && drawh != -1) 
                        && (x > drawx && y > drawy && x < drawx + draww && y < drawy + drawh))
                        || ((w > 0 && h > 0 && x < e.ClipRectangle.Width && y < e.ClipRectangle.Height)
                        && !(drawx != -1 && drawy != -1 && draww != -1 && drawh != -1))) {
                        RectangleF r = new RectangleF(x, y, bsize, bsize);
                        SolidBrush b = new SolidBrush(t.Color);
                        g.FillRectangle(b, r);
                        if (grid)
                            g.DrawRectangle(p, r.X, r.Y, r.Width, r.Height);
                        boxes++;
                        b.Dispose();
                    }
                }
                // Tags
                if (drawx == -1 && drawy == -1 && draww == -1 && drawh == -1) {
                    foreach (Tile t in tiles.Values) {
                        float x = mapOffsetX + (bsize * (float)t.X);
                        float y = mapOffsetY + (bsize * (float)t.Y);
                        float w = x + bsize;
                        float h = y + bsize;
                        // only draw what's in the viewport (0, 0, width, height)
                        if (w > 0 && h > 0 && x < e.ClipRectangle.Width && y < e.ClipRectangle.Height) {
                            SolidBrush b = new SolidBrush(Color.Black);
                            int tagCount = 0;
                            foreach (MapTag tag in t.Tags) {
                                if (ShowTagType(tag.Type)) {
                                    int len = tag.Tag.Length * 10;
                                    float lx1 = x + (bsize / 2);
                                    float lx2 = w + ld;
                                    float tx = w + ld;
                                    if (lx2 + len > map.ClientSize.Width) {
                                        lx2 = x - ld;
                                        tx = x - ld - len;
                                    }
                                    float ly1 = y + (bsize / 2);
                                    float ly2 = y - ld;
                                    float ty = y - ld - 10;
                                    if (ly2 < 0) {
                                        ly2 = h + ld;
                                        ty = h + ld - 10;
                                    }
                                    g.DrawLine(p, lx1, ly1, lx2, ly2);
                                    RectangleF r = new RectangleF(tx, ty, len, 20);
                                    g.DrawRectangle(p, r.X, r.Y, r.Width, r.Height);
                                    g.FillRectangle(wb, r);
                                    Font f = new Font(FontFamily.GenericSansSerif, 10.0f);
                                    g.DrawString(tag.Tag, f, b, tx, ty + (tagCount * 10));
                                    tagCount++;
                                }
                            }
                            b.Dispose();
                        }
                    }
                }
                tbBoxesDrawn.Text = boxes.ToString();
                p.Dispose();
                Graphics Viewable = map.CreateGraphics();
                Viewable.DrawImageUnscaled(BackBuffer, 0, 0);
                drawx = -1;
                drawy = -1;
                drawh = -1;
                draww = -1;
                g.Dispose();
            }
        }

        private void map_MouseEnter(object sender, EventArgs e) {
            mouseOverMap = true;
        }

        private void map_MouseMove(object sender, MouseEventArgs e) {
            tbX.Text = ((int)((e.X - mapOffsetX) / bsize)).ToString();
            tbY.Text = ((int)((e.Y - mapOffsetY) / bsize)).ToString();
            if (e.Button == MouseButtons.Right) {
                float dX = (float)e.X - dragX;
                float dY = (float)e.Y - dragY;
                mapOffsetX += dX;
                mapOffsetY += dY;
                //                tbTest.Text = "mapOffsetX (" + mapOffsetX.ToString() + ") + mapWidth * bsize (" + mapWidth.ToString() + " * " + bsize.ToString() + " = " + (mapWidth * bsize).ToString() + ") < ClientSize.Width (" + ClientSize.Width.ToString() + ")?";
                if (mapOffsetX < map.ClientSize.Width - ((mapWidth - 1) * bsize))
                    mapOffsetX = (int)(map.ClientSize.Width - (mapWidth * bsize));
                if (mapOffsetY < map.ClientSize.Height - ((mapHeight - 1) * bsize))
                    mapOffsetY = (int)(map.ClientSize.Height - (mapHeight * bsize));
                if (mapOffsetX > 0)
                    mapOffsetX = 0;
                if (mapOffsetY > 0)
                    mapOffsetY = 0;
                // let's only redraw at a certain framerate
                DateTime now = DateTime.Now;
                int fps = 10;
                TimeSpan delta = now - dragStartTime;
                if (delta.Milliseconds % (1000 / fps) != delta.Milliseconds) {
                    map.Refresh();
                    dragStartTime = now;  // we should substract the time passed, but what the hell...
                }
                dragX = e.X;
                dragY = e.Y;
            } else if (e.Button == MouseButtons.Left) {
                MouseDraw(e.X, e.Y);
            }

        }

        private void map_MouseLeave(object sender, EventArgs e) {
            mouseOverMap = false;
            dragging = false;
        }

        private void map_MouseDown(object sender, MouseEventArgs e) {
            if (e.Button == MouseButtons.Left) {
                dragging = false;
            } else if (e.Button == MouseButtons.Right) {
                dragging = true;
                dragX = e.X;
                dragY = e.Y;
                dragStartTime = DateTime.Now;
            }
        }

        private void map_MouseUp(object sender, MouseEventArgs e) {
            if (e.Button == MouseButtons.Left) {
                if (!dragging) {
                    MouseDraw(e.X, e.Y);
                }
                dragging = false;
            }
        }

        private void MouseDraw(int mouseX, int mouseY) {
            int x = (int)((mouseX - mapOffsetX) / bsize);
            int y = (int)((mouseY - mapOffsetY) / bsize);
            drawx = mouseX - (int)bsize;
            drawy = mouseY - (int)bsize;
            draww = (int)bsize * 2;
            drawh = (int)bsize * 2;
            if (state == State.Editing)
                DrawOnMap(x, y);
            else if (state == State.PlacingLand)
                PlaceLand(x, y);
            else if (state == State.MovingLand)
                MoveLand(x, y);
            else if (state == State.PlacingRegion)
                PlaceRegion(x, y);
            else if (state == State.MovingRegion)
                MoveRegion(x, y);
        }

        private void map_MouseWheel(object sender, MouseEventArgs e) {
            if (mouseOverMap) {
                int d = e.Delta / 120;
                int x = e.X - map.Location.X;
                int y = e.Y - map.Location.Y;
                float cX = Math.Abs(mapOffsetX) + x;
                float cY = Math.Abs(mapOffsetY) + y;
                int nX = (int)(cX / bsize);
                int nY = (int)(cY / bsize);
                mapOffsetX -= (int)(d * nX * bsize * 0.1f);
                mapOffsetY -= (int)(d * nY * bsize * 0.1f);

                // change zoom by 10% per increment of the mousewheel
                // size 10 is smallest, 100 is biggest
                if (d < 0)
                    bsize *= Math.Abs(d) * 0.9f;
                else if (d > 0)
                    bsize *= d * 1.1f;
                if (bsize < 1)
                    bsize = 1;
                if (bsize > 100)
                    bsize = 100;
                float minBsizeX = (float)map.ClientSize.Width / (float)mapWidth;
                float minBsizeY = (float)map.ClientSize.Height / (float)mapHeight;
                float minBsize = (minBsizeX > minBsizeY ? minBsizeX : minBsizeY);
                if ((mapWidth * bsize) < map.ClientSize.Width
                    || (mapHeight * bsize) < map.ClientSize.Height)
                    bsize = minBsize;
                //                mapOffsetX -= (int)(d * bsize * 0.1f);
                //                mapOffsetY -= (int)(d * bsize * 0.1f);
                if (mapOffsetX > 0)
                    mapOffsetX = 0;
                if (mapOffsetY > 0)
                    mapOffsetY = 0;
                map.Refresh();
            }
        }

        private void UpdateEditBoxes() {
            pnlForest.BorderStyle = BorderStyle.None;
            pnlPlain.BorderStyle = BorderStyle.None;
            pnlMountain.BorderStyle = BorderStyle.None;
            pnlDesert.BorderStyle = BorderStyle.None;
            pnlSwamp.BorderStyle = BorderStyle.None;
            pnlRandom.BorderStyle = BorderStyle.None;
            pnlOcean.BorderStyle = BorderStyle.None;
            pnlSealane.BorderStyle = BorderStyle.None;
            pnlPortCity.BorderStyle = BorderStyle.None;
            pnlForest.BackgroundImage = null;
            pnlPlain.BackgroundImage = null;
            pnlMountain.BackgroundImage = null;
            pnlDesert.BackgroundImage = null;
            pnlSwamp.BackgroundImage = null;
            pnlRandom.BackgroundImage = null;
            pnlOcean.BackgroundImage = null;
            pnlSealane.BackgroundImage = null;
            pnlPortCity.BackgroundImage = null;
            switch (boxSelected) { 
                case 1:
                    pnlForest.BorderStyle = BorderStyle.FixedSingle;
                    pnlForest.BackgroundImage = mapgui.Properties.Resources.dot;
                    break;
                case 2:
                    pnlPlain.BorderStyle = BorderStyle.FixedSingle;
                    pnlPlain.BackgroundImage = mapgui.Properties.Resources.dot;
                    break;
                case 3:
                    pnlMountain.BorderStyle = BorderStyle.FixedSingle;
                    pnlMountain.BackgroundImage = mapgui.Properties.Resources.dot;
                    break;
                case 4:
                    pnlDesert.BorderStyle = BorderStyle.FixedSingle;
                    pnlDesert.BackgroundImage = mapgui.Properties.Resources.dot;
                    break;
                case 5:
                    pnlSwamp.BorderStyle = BorderStyle.FixedSingle;
                    pnlSwamp.BackgroundImage = mapgui.Properties.Resources.dot;
                    break;
                case 6:
                    pnlRandom.BorderStyle = BorderStyle.FixedSingle;
                    pnlRandom.BackgroundImage = mapgui.Properties.Resources.dot;
                    break;
                case 7:
                    pnlOcean.BorderStyle = BorderStyle.FixedSingle;
                    pnlOcean.BackgroundImage = mapgui.Properties.Resources.dot;
                    break;
                case 8:
                    pnlSealane.BorderStyle = BorderStyle.FixedSingle;
                    pnlSealane.BackgroundImage = mapgui.Properties.Resources.dot;
                    break;
                case 9:
                    pnlPortCity.BorderStyle = BorderStyle.FixedSingle;
                    pnlPortCity.BackgroundImage = mapgui.Properties.Resources.dot;
                    break;
            }
        }

        private void pnlForest_MouseClick(object sender, MouseEventArgs e) {
            if (boxSelected == 1) {
                boxSelected = 0;
                state = State.Idle;
            }
            else {
                boxSelected = 1;
                state = State.Editing;
            }
            UpdateEditBoxes();
        }

        private void pnlPlain_MouseClick(object sender, MouseEventArgs e) {
            if (boxSelected == 2) {
                boxSelected = 0;
                state = State.Idle;
            }
            else {
                boxSelected = 2;
                state = State.Editing;
            }
            UpdateEditBoxes();
        }

        private void pnlMountain_MouseClick(object sender, MouseEventArgs e) {
            if (boxSelected == 3) {
                boxSelected = 0;
                state = State.Idle;
            }
            else {
                boxSelected = 3;
                state = State.Editing;
            }
            UpdateEditBoxes();
        }

        private void pnlDesert_MouseClick(object sender, MouseEventArgs e) {
            if (boxSelected == 4) {
                boxSelected = 0;
                state = State.Idle;
            }
            else {
                boxSelected = 4;
                state = State.Editing;
            }
            UpdateEditBoxes();
        }

        private void pnlSwamp_MouseClick(object sender, MouseEventArgs e) {
            if (boxSelected == 5) {
                boxSelected = 0;
                state = State.Idle;
            }
            else {
                boxSelected = 5;
                state = State.Editing;
            }
            UpdateEditBoxes();
        }

        private void pnlRandom_MouseClick(object sender, MouseEventArgs e) {
            if (boxSelected == 6) {
                boxSelected = 0;
                state = State.Idle;
            }
            else {
                boxSelected = 6;
                state = State.Editing;
            }
            UpdateEditBoxes();
        }

        private void pnlOcean_MouseClick(object sender, MouseEventArgs e) {
            if (boxSelected == 7) {
                boxSelected = 0;
                state = State.Idle;
            }
            else {
                boxSelected = 7;
                state = State.Editing;
            }
            UpdateEditBoxes();
        }

        private void pnlSealane_MouseClick(object sender, MouseEventArgs e) {
            if (boxSelected == 8) {
                boxSelected = 0;
                state = State.Idle;
            } else {
                boxSelected = 8;
                state = State.Editing;
            }
            UpdateEditBoxes();
        }

        private void pnlPortCity_MouseClick(object sender, MouseEventArgs e) {
            if (boxSelected == 9) {
                boxSelected = 0;
                state = State.Idle;
            } else {
                boxSelected = 9;
                state = State.Editing;
            }
            UpdateEditBoxes();
        }

        private void LandSelected() {
            if (lbLands.SelectedIndex > -1) {
                if (lbLands.SelectedIndex != lastSelectedLand) {
                    btnPlaceMoveLand.Enabled = true;
                    btnRemoveLand.Enabled = true;
                    btnEditLand.Enabled = true;
                    Land l = (Land)lbLands.SelectedItem;
                    if (l.X > -1 && l.Y > -1) {
                        // Suggest to move it
                        btnPlaceMoveLand.Text = "Move";
                    } else {
                        // Suggest to place it
                        btnPlaceMoveLand.Text = "Place";
                    }
                    state = State.Idle;
                }
            } else { 
                // Reset
                btnPlaceMoveLand.Enabled = false;
                btnRemoveLand.Enabled = false;
                btnEditLand.Enabled = false;
                btnPlaceMoveLand.Text = "Place";
                state = State.Idle;
            }
            lastSelectedLand = lbLands.SelectedIndex;
        }

        private void lbLands_SelectedIndexChanged(object sender, EventArgs e) {
            LandSelected();
        }

        private void tcLists_SelectedIndexChanged(object sender, EventArgs e) {
            TabPage tab = ((TabControl)sender).SelectedTab;
            if (tab.Equals(tabEdit)) {
                if (boxSelected > 0)
                    state = State.Editing;
            } else if (tab.Equals(tabLands)) {
                state = State.Idle;
                LandSelected();
            } else if (tab.Equals(tabRegions)) {
                state = State.Idle;
                RegionSelected();
            } else if (tab.Equals(tabHavens)) {
                state = State.Idle;
                HavenSelected();
            } else {
                state = State.Idle;
            }
        }

        private void lbRegions_SelectedIndexChanged(object sender, EventArgs e) {
            RegionSelected();
        }

        private void RegionSelected() {
            if (lbRegions.SelectedIndex > -1) {
                if (lbRegions.SelectedIndex != lastSelectedRegion) {
                    btnPlaceMoveRegion.Enabled = true;
                    btnRemoveRegion.Enabled = true;
                    btnEditRegion.Enabled = true;
                    Region r = (Region)lbRegions.SelectedItem;
                    if (r.X > -1 && r.Y > -1) {
                        // Suggest to move it
                        btnPlaceMoveRegion.Text = "Move";

                    } else {
                        // Suggest to place it
                        btnPlaceMoveRegion.Text = "Place";
                    }
                    state = State.Idle;
                }
            } else { 
                // Reset
                btnPlaceMoveRegion.Enabled = false;
                btnRemoveRegion.Enabled = false;
                btnEditRegion.Enabled = false;
                btnPlaceMoveRegion.Text = "Place";
                state = State.Idle;
            }
            lastSelectedRegion = lbRegions.SelectedIndex;
        }

        private void btnPlaceMoveRegion_Click(object sender, EventArgs e) {
            if (btnPlaceMoveRegion.Text.Equals("Place")) {
                state = State.PlacingRegion;
                btnPlaceMoveRegion.Text = "Stop";
            } else if (btnPlaceMoveRegion.Text.Equals("Move")) {
                state = State.MovingRegion;
                btnPlaceMoveRegion.Text = "Stop";
            } else { // assuming "Stop"
                state = State.Idle;
                lbRegions.SelectedIndex = -1;
            }
        }

        private void btnPlaceMoveLand_Click(object sender, EventArgs e) {
            if (btnPlaceMoveLand.Text.Equals("Place")) {
                state = State.PlacingLand;
                btnPlaceMoveLand.Text = "Stop";
            } else if (btnPlaceMoveLand.Text.Equals("Move")) {
                state = State.MovingLand;
                btnPlaceMoveLand.Text = "Stop";
            } else { // assuming "Stop"
                state = State.Idle;
                lbLands.SelectedIndex = -1;
            }
        }

        private void HavenSelected() {
        
        }

        private bool CheckMapConsistency() { 
            // Check if Lands are on land tiles
            foreach (Land l in lands) {
                Coord xy = new Coord(l.X, l.Y);
                Tile t = tiles[xy];
                if (!t.IsLand()) {
                    MessageBox.Show(this, "One or more of the lands are not located on land tiles.");
                    return false;
                }
            }
            // TODO: Other checks
            return true;
        }

        private void btnRemoveLand_Click(object sender, EventArgs e) {
            if (MessageBox.Show(this, "Are you sure you wish to remove this Land?", "Remove Land", MessageBoxButtons.YesNo) == DialogResult.Yes) {
                Land l = (Land)lbLands.SelectedItem;
                lbLands.Items.Remove(l);
                lands.Remove(l);
                MapTag tag = new MapTag(l.Name, TagType.Land);
                Coord xy = new Coord(l.X, l.Y);
                tiles[xy].Tags.Remove(tag);
            }
            map.Refresh();
        }

        private void btnSaveLand_Click(object sender, EventArgs e) {
            if (!tbLandName.Text.Trim().Equals("")) {
                if (lbLands.SelectedIndex == -1) {
                    bool exists = false;
                    foreach (Land l in lands) {
                        if (l.Name.Equals(tbLandName.Text.Trim())) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        Land newLand = new Land(-1, -1, ' ', tbLandName.Text.Trim());
                        lands.Add(newLand);
                        lbLands.Items.Add(newLand);
                    }
                } else {
                    Land editLand = (Land)lbLands.SelectedItem;
                    foreach (Land l in lands) {
                        if (l.Equals(editLand)) {
                            MapTag tag = new MapTag(l.Name, TagType.Land);
                            Coord xy = new Coord(l.X, l.Y);
                            if (tiles.ContainsKey(xy)) {
                                foreach (MapTag t in tiles[xy].Tags)
                                    if (t.Equals(tag)) {
                                        t.Tag = tbLandName.Text.Trim();
                                        break;
                                    }
                            }
                            l.Name = tbLandName.Text.Trim();
                            break;
                        }
                    }
                }
                UpdateLandList();
                map.Refresh();
            }
            btnSaveLand.Enabled = false;
            tbLandName.Enabled = false;
            tbLandName.Text = "";
        }

        private void btnNewLand_Click(object sender, EventArgs e) {
            lbLands.SelectedIndex = -1;
            btnSaveLand.Enabled = true;
            tbLandName.Enabled = true;
            tbLandName.Text = "";
        }

        private void btnEditLand_Click(object sender, EventArgs e) {
            btnSaveLand.Enabled = true;
            tbLandName.Enabled = true;
            tbLandName.Text = ((Land)lbLands.SelectedItem).Name;
        }

        private void btnNewRegion_Click(object sender, EventArgs e) {
            lbRegions.SelectedIndex = -1;
            btnSaveRegion.Enabled = true;
            tbRegionName.Enabled = true;
            tbRegionName.Text = "";
        }

        private void btnEditRegion_Click(object sender, EventArgs e) {
            btnSaveRegion.Enabled = true;
            tbRegionName.Enabled = true;
            tbRegionName.Text = ((Region)lbRegions.SelectedItem).Name;
        }

        private void btnSaveRegion_Click(object sender, EventArgs e) {
            if (!tbRegionName.Text.Trim().Equals("")) {
                if (lbRegions.SelectedIndex == -1) {
                    bool exists = false;
                    foreach (Region r in regions) {
                        if (r.Name.Equals(tbRegionName.Text.Trim())) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        Region newRegion = new Region(-1, -1, tbRegionName.Text.Trim());
                        regions.Add(newRegion);
                        lbRegions.Items.Add(newRegion);
                    }
                } else {
                    Region editRegion = (Region)lbRegions.SelectedItem;
                    foreach (Region r in regions) {
                        if (r.Equals(editRegion)) {
                            MapTag tag = new MapTag(r.Name, TagType.Region);
                            Coord xy = new Coord(r.X, r.Y);
                            if (tiles.ContainsKey(xy)) {
                                foreach (MapTag t in tiles[xy].Tags)
                                    if (t.Equals(tag)) {
                                        t.Tag = tbRegionName.Text.Trim();
                                        break;
                                    }
                            }
                            r.Name = tbRegionName.Text.Trim();
                            break;
                        }
                    }
                }
                UpdateRegionList();
                map.Refresh();
            }
            btnSaveRegion.Enabled = false;
            tbRegionName.Enabled = false;
            tbRegionName.Text = "";
        }

        private void lbHavens_SelectedIndexChanged(object sender, EventArgs e) {
            HavenSelected();
        }

        private void pnlHaven1_MouseClick(object sender, MouseEventArgs e) {

        }

        private void pnlHaven2_MouseClick(object sender, MouseEventArgs e) {

        }

        private void pnlHaven3_MouseClick(object sender, MouseEventArgs e) {

        }

        private void pnlHaven4_MouseClick(object sender, MouseEventArgs e) {

        }

        private void pnlHaven5_MouseClick(object sender, MouseEventArgs e) {

        }

        private void pnlHaven6_MouseClick(object sender, MouseEventArgs e) {

        }

        private void MapGUI_FormClosing(object sender, FormClosingEventArgs e) {
            if (autoLoadSaveMap) {
                if (CheckMapConsistency() && !mapFile.Equals(""))
                    SaveMap();
                else {
                    e.Cancel = true;
                }
            }
        }

  







    }

}
