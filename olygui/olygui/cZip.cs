using System;
using ICSharpCode.SharpZipLib.Zip;
using System.IO;
using System.Collections.Generic;
using System.Collections;

namespace olygui {
    public class cZip {
        public cZip() { }

        public static void ZipFolder(string pPath, string outPathAndZipFile, string password) {
            string OutPath = outPathAndZipFile;
            ArrayList ar = GenerateFileList(pPath); // generate file list
            // find number of chars to remove from orginal file path

            string tParent = (Directory.GetParent(pPath)).ToString();
            int TrimLength = tParent.Length;
            if (tParent.Substring(tParent.Length - 1, 1) != "\\")
                TrimLength += 1; //remove '\'

            FileStream ostream;
            byte[] obuffer;
            ZipOutputStream oZipStream = new ZipOutputStream(System.IO.File.Create(OutPath)); // create zip stream
            if (password != String.Empty) oZipStream.Password = password;
            oZipStream.SetLevel(9); // 9 = maximum compression level
            ZipEntry oZipEntry;
            foreach (string Fil in ar) // for each file, generate a zipentry
            {
                oZipEntry = new ZipEntry(Fil.Remove(0, TrimLength));
                oZipStream.PutNextEntry(oZipEntry);

                if (!Fil.EndsWith(@"/")) // if a file ends with '/' its a directory
                {
                    ostream = File.OpenRead(Fil);
                    obuffer = new byte[ostream.Length]; // byte buffer
                    ostream.Read(obuffer, 0, obuffer.Length);
                    oZipStream.Write(obuffer, 0, obuffer.Length);
                    Console.Write(".");
                    ostream.Close();
                }
            }
            oZipStream.Finish();
            oZipStream.Close();
        }

        public static void ZipFile(string pFile, string outPathAndZipFile, string password) {
            string OutPath = outPathAndZipFile;
            ArrayList ar = new ArrayList();
            ar.Add(pFile);
            // find number of chars to remove from orginal file path
            string tParent = (Directory.GetParent(pFile)).ToString();
            int TrimLength = tParent.Length;
            if (tParent.Substring(tParent.Length - 1, 1) != "\\")
                TrimLength += 1; //remove '\'
            FileStream ostream;
            byte[] obuffer;
            ZipOutputStream oZipStream = new ZipOutputStream(System.IO.File.Create(OutPath)); // create zip stream
            if (password != String.Empty) oZipStream.Password = password;
            oZipStream.SetLevel(9); // 9 = maximum compression level
            ZipEntry oZipEntry;
            foreach (string Fil in ar) // for each file, generate a zipentry
            {
                oZipEntry = new ZipEntry(Fil.Remove(0, TrimLength));
                oZipStream.PutNextEntry(oZipEntry);

                if (!Fil.EndsWith(@"/")) // if a file ends with '/' its a directory
                {
                    ostream = File.OpenRead(Fil);
                    obuffer = new byte[ostream.Length]; // byte buffer
                    ostream.Read(obuffer, 0, obuffer.Length);
                    oZipStream.Write(obuffer, 0, obuffer.Length);
                    Console.Write(".");
                    ostream.Close();
                }
            }
            oZipStream.Finish();
            oZipStream.Close();
        }

        public static void UnZipFiles(string zipPathAndFile, string outputFolder, string password, bool deleteZipFile) {
            ZipInputStream s = new ZipInputStream(File.OpenRead(zipPathAndFile));
            if (password != null && password != String.Empty)
                s.Password = password;
            ZipEntry theEntry;
            string tmpEntry = String.Empty;
            while ((theEntry = s.GetNextEntry()) != null) {
                string directoryName = outputFolder;
                string fileName = Path.GetFileName(theEntry.Name);
                // create directory 
                if (directoryName != "") {
                    Directory.CreateDirectory(directoryName);
                }
                if (fileName != String.Empty) {
                    if (theEntry.Name.IndexOf(".ini") < 0) {
                        string fullPath = directoryName + "\\" + theEntry.Name;
                        fullPath = fullPath.Replace("\\ ", "\\");
                        string fullDirPath = Path.GetDirectoryName(fullPath);
                        if (!Directory.Exists(fullDirPath)) Directory.CreateDirectory(fullDirPath);
                        FileStream streamWriter = File.Create(fullPath);
                        int size = 2048;
                        byte[] data = new byte[2048];
                        while (true) {
                            size = s.Read(data, 0, data.Length);
                            if (size > 0) {
                                streamWriter.Write(data, 0, size);
                            } else {
                                break;
                            }
                        }
                        streamWriter.Close();
                    }
                }
            }
            s.Close();
            if (deleteZipFile)
                File.Delete(zipPathAndFile);
        }

        private static ArrayList GenerateFileList(string Dir) {
            ArrayList mid = new ArrayList();
            bool Empty = true;
            foreach (string file in Directory.GetFiles(Dir)) // add each file in directory
            {
                mid.Add(file);
                Empty = false;
            }

            if (Empty) {
                if (Directory.GetDirectories(Dir).Length == 0) // if directory is completely empty, add it
                {
                    mid.Add(Dir + @"/");
                }
            }
            foreach (string dirs in Directory.GetDirectories(Dir)) // do this recursively
            {
                // set up the excludeDir test
                string testDir = dirs.Substring(dirs.LastIndexOf(@"\") + 1).ToUpper();
                foreach (object obj in GenerateFileList(dirs)) {
                    mid.Add(obj);
                }
            }
            return mid; // return file list
        }
    }
}
