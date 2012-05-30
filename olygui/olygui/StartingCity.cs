using System;
using System.Collections.Generic;
using System.Text;

namespace olygui {

    public class StartingCity {

        public StartingCity() { 
            
        }

        public StartingCity(int idx, int id, string name) {
            this.idx = idx;
            this.id = id;
            this.name = name;
        }

        public int idx = 0;
        public int id = 0;
        public string name = "";

        public override string ToString() {
            return name;
        }
    }

}
