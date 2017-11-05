using Microsoft.AspNet.SignalR;
using Microsoft.Owin.Cors;
using Owin;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SignalRServerSample
{
    class Startup
    {
        public void Configuration(IAppBuilder app)
        {
            app.UseErrorPage();
       
            app.Map("/signalr", map =>
            {
                var config = new HubConfiguration
                {
                    // You can enable JSONP by uncommenting this line
                    // JSONP requests are insecure but some older browsers (and some
                    // versions of IE) require JSONP to work cross domain
                    // EnableJSONP = true
                };

                // Turns cors support on allowing everything
                // In real applications, the origins should be locked down
                map.UseCors(CorsOptions.AllowAll)
                   .RunSignalR(config);
            });

            // Turn tracing on programmatically
            GlobalHost.TraceManager.Switch.Level = SourceLevels.All;
        }
    }
}
