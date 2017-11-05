using Microsoft.AspNet.SignalR;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SignalRServerSample.Hubs
{
    public class Chat : Hub
    {
        private static float value = 0.1f;
        public void Send(string message)
        {
            Clients.All.send(DateTime.Now +" " + message);
        }
        public void SendFloat(string message)
        {
            Clients.All.sendFloat(8.0);

            Console.WriteLine(value);

            value *= new Random().Next(0, 100);

        }
    }
}
