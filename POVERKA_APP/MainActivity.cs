using System;
using Android.App;
using Android.OS;
using Android.Runtime;
using Android.Views;
using AndroidX.AppCompat.Widget;
using AndroidX.AppCompat.App;
using Google.Android.Material.FloatingActionButton;
using Google.Android.Material.Snackbar;
using ApxLabs.FastAndroidCamera;
using Android.Hardware;

namespace POVERKA_APP
{
    [Activity(Label = "@string/app_name", Theme = "@style/AppTheme.NoActionBar", MainLauncher = true)]
    public class MainActivity : AppCompatActivity
    {
        protected override void OnCreate(Bundle savedInstanceState)
        {
            //Camera camera = Camera.Open();
            //Camera.Parameters parameters = camera.GetParameters();

            //// snip - set resolution, frame rate, preview format, etc.

            //camera.SetParameters(parameters);

            //// assuming the SurfaceView has been set up elsewhere
            //camera.SetPreviewDisplay(_surfaceView.Holder);
            //camera.StartPreview();

            //int numBytes = (parameters.PreviewSize.Width * parameters.PreviewSize.Height * ImageFormat.GetBitsPerPixel(parameters.PreviewFormat)) / 8;
            //for (uint i = 0; i < NUM_PREVIEW_BUFFERS; ++i)
            //{
            //    using (FastJavaByteArray buffer = new FastJavaByteArray(numBytes))
            //    {
            //        // allocate new Java byte arrays for Android to use for preview frames
            //        camera.AddCallbackBuffer(new FastJavaByteArray(numBytes));
            //    }
            //    // The using block automatically calls Dispose() on the buffer, which is safe
            //    // because it does not automaticaly destroy the Java byte array. It only releases
            //    // our JNI reference to that array; the Android Camera (in Java land) still
            //    // has its own reference to the array.
            //}

            //// non-marshaling version of the preview callback
            //camera.SetNonMarshalingPreviewCallback(this);

           
            base.OnCreate(savedInstanceState);
            Xamarin.Essentials.Platform.Init(this, savedInstanceState);
            SetContentView(Resource.Layout.activity_main);

            Toolbar toolbar = FindViewById<Toolbar>(Resource.Id.toolbar);
            SetSupportActionBar(toolbar);

            FloatingActionButton fab = FindViewById<FloatingActionButton>(Resource.Id.fab);
            fab.Click += FabOnClick;
        }

        public override bool OnCreateOptionsMenu(IMenu menu)
        {
            MenuInflater.Inflate(Resource.Menu.menu_main, menu);
            return true;
        }

        public override bool OnOptionsItemSelected(IMenuItem item)
        {
            int id = item.ItemId;
            if (id == Resource.Id.action_settings)
            {
                return true;
            }

            return base.OnOptionsItemSelected(item);
        }

        private void FabOnClick(object sender, EventArgs eventArgs)
        {
            View view = (View) sender;
            Snackbar.Make(view, "Replace with your own action", Snackbar.LengthLong)
                .SetAction("Action", (View.IOnClickListener)null).Show();
        }

        public override void OnRequestPermissionsResult(int requestCode, string[] permissions, [GeneratedEnum] Android.Content.PM.Permission[] grantResults)
        {
            Xamarin.Essentials.Platform.OnRequestPermissionsResult(requestCode, permissions, grantResults);

            base.OnRequestPermissionsResult(requestCode, permissions, grantResults);
        }
	}
}
