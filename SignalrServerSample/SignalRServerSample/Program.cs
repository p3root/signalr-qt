using Microsoft.Owin.Hosting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SignalRServerSample
{
    class Program
    {
        static void Main(string[] args)
        {
            using (WebApp.Start<Startup>("http://192.168.8.119:8080"))
            {
                Console.WriteLine("Server running at http://192.168.8.119:8080/");
                Console.ReadLine();
            }
        }
    }
}
