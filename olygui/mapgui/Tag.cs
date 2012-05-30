using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace mapgui {

    public enum TagType { 
        Unknown = 0,
        Land    = 1,
        Region  = 2,
        Haven   = 3
    }

    public class MapTag {

        private string tag;
        private TagType type;

        public string Tag { get { return tag; } set { tag = value; } }
        public TagType Type { get { return type; } set { type = value; } }

        public MapTag() {
            tag = "";
            type = TagType.Unknown;
        }

        public MapTag(string tag, TagType type) : this() {
            this.tag = tag;
            this.type = type;
        }

        public override bool Equals(object obj) {
            if (obj == null || GetType() != obj.GetType())
                return false;
            MapTag o = (MapTag)obj;
            return (o.type.Equals(type) && o.tag.Equals(tag));
        }

    }

}
