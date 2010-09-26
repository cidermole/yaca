using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace ConsoleApplication1
{
    class Program
    {
        [DllImport("coredll.dll")]
        public static extern IntPtr FindWindow(String sClassName, String sAppName);

        [DllImport("coredll.dll")]
        public static extern bool DestroyWindow(IntPtr hwnd);

        static void Main(string[] args)
        {
            IntPtr hwnd;

            // Verbinden nicht möglich, Browser Warning
            while (true)
            {
                hwnd = FindWindow(null, "Verbinden nicht möglich");
                if (hwnd != null)
                    DestroyWindow(hwnd);
                hwnd = FindWindow(null, "Browser Warning");
                if (hwnd != null)
                    DestroyWindow(hwnd);
                System.Threading.Thread.Sleep(1000);
            }
        }
    }
}

