namespace mapgui {

    public class Coord {

        public int x = 0;
        public int y = 0;

        public Coord(int x, int y) {
            this.x = x;
            this.y = y;
        }

        public override bool Equals(object obj) {
            if (obj == null || GetType() != obj.GetType())
                return false;
            Coord o = (Coord)obj;
            return (o.x.Equals(x) && o.y.Equals(y));
        }

        public override int GetHashCode() {
            return x * 1000000 + y;
        }
    }

}
