using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Drawing;
using System.Text;
using System.Threading.Tasks;

namespace ManagedLib
{
    public static unsafe class Entrance
    {
        [UnmanagedCallersOnly]
        public static void Init(InitPayload payload)
        {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("----- Entrance Method Called! -----");
            Console.ForegroundColor = ConsoleColor.White;

            Component.InitDelegates(payload);

            test();

            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine();
            Console.WriteLine("----- Entrance Method Exiting! -----");
            Console.ForegroundColor = ConsoleColor.White;
        }

        private static void test()
        {
            for (var i = 0; i < 3; i++)
            {
                using var comp = new Component(i, $"My-tag-for-#{i}");
                Console.WriteLine($"\tThis is #{comp.Id} Component speaking");
                Console.WriteLine($"\tMy tag is: {comp.Tag}");
            }
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct InitPayload
    {
        public delegate* unmanaged<int, IntPtr, IntPtr> nativeComponentNew;
        public delegate* unmanaged<IntPtr, void> nativeComponentDelete;
        public delegate* unmanaged<IntPtr, int, void> nativeComponentSetId;
        public delegate* unmanaged<IntPtr, int> nativeComponentGetId;
        public delegate* unmanaged<IntPtr, IntPtr, void> nativeComponentSetTag;
        public delegate* unmanaged<IntPtr, IntPtr> nativeComponentGetTag;
    }
}
