using System;
using System.Text;
using System.IO;

namespace entab {

    class Program {

        static void Main(string[] args) {
            if (args.Length >=1 && args.Length <= 2) { 
                string inputfile = args[0];
                string outputfile = inputfile;
                if (args.Length == 2)
                    outputfile = args[1];
                if (File.Exists(inputfile)) {
                    StreamReader sr = new StreamReader(inputfile);
                    string txt = sr.ReadToEnd();
                    sr.Close();
                    txt.Replace("    ", "\t");
                    StreamWriter sw = new StreamWriter(outputfile, false);
                    sw.Write(txt);
                    sw.Close();
                } else {
                    Console.WriteLine("Error: inputfile doesn't exist.");
                }
            } else {
                Console.WriteLine("Usage: entab inputfile outputfile");
            }
        }

    }

}
