using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;

using Kitware.VTK;

namespace ActiViz.Examples {
   public partial class Form1 : Form {
      public Form1() {
         InitializeComponent();
      }


      private void renderWindowControl1_Load(object sender, EventArgs e) {
         try {
            ReadStructuredGrid();
         }
         catch(Exception ex) {
            MessageBox.Show(ex.Message, "Exception", MessageBoxButtons.OK);
         }
      }


      private void ReadStructuredGrid() {
         // Path to vtk data must be set as an environment variable
         // VTK_DATA_ROOT = "C:\VTK\vtkdata-5.8.0"
         vtkTesting test = vtkTesting.New();
         string root = test.GetDataRoot();
         string filePath = System.IO.Path.Combine(root, @"Data\multicomb_0.vts");

         // reader
         vtkXMLStructuredGridReader reader = vtkXMLStructuredGridReader.New();
         if(reader.CanReadFile(filePath) == 0) {
            MessageBox.Show("Cannot read file \"" + filePath + "\"", "Error", MessageBoxButtons.OK);
            return;
         }
         reader.SetFileName(filePath);
         reader.Update(); // here we read the file actually

         vtkStructuredGridGeometryFilter geometryFilter = vtkStructuredGridGeometryFilter.New();
         geometryFilter.SetInputConnection(reader.GetOutputPort());
         geometryFilter.Update();

         // Visualize
         vtkPolyDataMapper mapper = vtkPolyDataMapper.New();
         mapper.SetInputConnection(geometryFilter.GetOutputPort());

         //// mapper
         //vtkDataSetMapper mapper = vtkDataSetMapper.New();
         //mapper.SetInputConnection(reader.GetOutputPort());

         // actor
         vtkActor actor = vtkActor.New();
         actor.SetMapper(mapper);
         // get a reference to the renderwindow of our renderWindowControl1
         vtkRenderWindow renderWindow = renderWindowControl1.RenderWindow;
         // renderer
         vtkRenderer renderer = renderWindow.GetRenderers().GetFirstRenderer();
         // set background color
         renderer.SetBackground(0.2, 0.3, 0.4);
         // add our actor to the renderer
         renderer.AddActor(actor);
      }
   }
}
