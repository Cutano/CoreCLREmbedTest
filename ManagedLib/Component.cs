using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ManagedLib
{
    public unsafe class Component : IDisposable
    {
        private IntPtr _nativeHandle = IntPtr.Zero;
        private static delegate* unmanaged<int, IntPtr, IntPtr> nativeComponentNew;
        private static delegate* unmanaged<IntPtr, void> nativeComponentDelete;
        private static delegate* unmanaged<IntPtr, int, void> nativeComponentSetId;
        private static delegate* unmanaged<IntPtr, int> nativeComponentGetId;
        private static delegate* unmanaged<IntPtr, IntPtr, void> nativeComponentSetTag;
        private static delegate* unmanaged<IntPtr, IntPtr> nativeComponentGetTag;

        private bool disposedValue;

        public int Id
        {
            set => nativeComponentSetId(_nativeHandle, value);
            get => nativeComponentGetId(_nativeHandle);
        }

        public string Tag
        {
            set => nativeComponentSetTag(_nativeHandle, Marshal.StringToBSTR(value));
            get => Marshal.PtrToStringUni(nativeComponentGetTag(_nativeHandle));
        }

        public Component(int id, string tag)
        {
            _nativeHandle = nativeComponentNew(id, Marshal.StringToBSTR(tag));
            if (_nativeHandle != IntPtr.Zero)
            {
                Console.WriteLine($"ComponentWrapper #{id} constructed in managed code with tag: {Tag}!";
            }
        }

        public static void InitDelegates(InitPayload payload)
        {
            nativeComponentNew = payload.nativeComponentNew;
            nativeComponentDelete = payload.nativeComponentDelete;
            nativeComponentSetId = payload.nativeComponentSetId;
            nativeComponentGetId = payload.nativeComponentGetId;
            nativeComponentSetTag = payload.nativeComponentSetTag;
            nativeComponentGetTag = payload.nativeComponentGetTag;
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    // TODO: 释放托管状态(托管对象)
                }

                // TODO: 释放未托管的资源(未托管的对象)并重写终结器
                Console.WriteLine($"ComponentWrapper #{Id} deconstructed in managed code!");
                nativeComponentDelete(_nativeHandle);
                _nativeHandle = IntPtr.Zero;
                // TODO: 将大型字段设置为 null
                disposedValue = true;
            }
        }

        // TODO: 仅当“Dispose(bool disposing)”拥有用于释放未托管资源的代码时才替代终结器
        ~Component()
        {
            // 不要更改此代码。请将清理代码放入“Dispose(bool disposing)”方法中
            Dispose(disposing: false);
        }

        public void Dispose()
        {
            // 不要更改此代码。请将清理代码放入“Dispose(bool disposing)”方法中
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }
}
