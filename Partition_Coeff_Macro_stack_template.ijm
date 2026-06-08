///Directory setup, modify as needed
///directory setup:
//parent/
//│
//├── child/
//│   │
//│   ├── raw_data_z/          ← original/raw experimental frames, make sure stack is in image sequence
//│   │     (e.g. .tif, .nd2, .czi, .lif, .ome.tiff etc.)
//│   │
//│   ├── analyzed/            ← processed versions of the raw data, this is where the dropelt detected image is store 
//│
//└── results/                 ← output of background and dropelt imformation for each frame
///
///If threshold method don't converge, it would output error message. This happens when droplets are not focus, causing 
/// the foreground(droplet) and background nearly identical 

rootDir = "Your Directory";
Date = "Modify as needed";
subDir = "Modify as needed";
Fnum = "Modify as needed";
inputDir = rootDir + Date + subDir + Fnum + "raw_data_z/";	///raw image data
outputDir = rootDir + Date + subDir + "results/";	///analyzed result store as .csv
analyzedDir = rootDir + Date + subDir + Fnum + "analyzed/";	///store dropelt detection image
///
list = getFileList(inputDir);
print(list.length);


for (i = 0; i < list.length; i++) {
    fileName = list[i];
    if (endsWith(fileName, ".tif") && startsWith(fileName, "Z-")) {	//filename must be in Z-nnn_nnn.tif format, where n is any digit from 0-9
    	print(fileName);
        filePath = inputDir + fileName;
        open(filePath);
        idOrig = getImageID();
        selectImage(idOrig);

        // Reset scale to pixels
        run("Set Scale...", "distance=1 known=1 unit=pixel");

        // --- Background Measurement ---
        run("Duplicate...", "title=Green_Background duplicate channels=1");
        run("Median...", "radius=2");	//smooth out noise
        setAutoThreshold("MinError dark");	//dark means background is dark --> foreground is bright
        setOption("BlackBackground", false);	//only works for binary morphology(Erode, watershed, etc.), does nothing in this part!
        run("Convert to Mask");					// bin image, foreground (dropelt) = white(255) and background = black(0)
        run("Invert");							// Flip foreground (dropelt) = 0, background = 255, also flip threshold range
        run("Create Selection");				// selection from WHITE areas → droplets
        selectImage(idOrig);					// go back to the original image
        run("Restore Selection");				// put the droplet ROI onto the original
        run("Clear Results");
        run("Set Measurements...", "area mean standard modal min bounding feret's area_fraction centroid rectangle limit display redirect=None decimal=3");	//limit doesn't do anything since the original image don't have threshold set
        run("Measure");

        // --- Droplet Measurement ---
        selectImage(idOrig);
    	run("Duplicate...", "title=Green_Droplets_Redirect duplicate channels=1");
    	selectImage(idOrig);
    	run("Duplicate...", "title=Green_Droplets duplicate channels=1");
    	run("Select None");
    	run("Median...", "radius=2");
        setAutoThreshold("Intermodes dark");
        setOption("BlackBackground", false);
        run("Convert to Mask");
    	run("Watershed");
//    	run("Erode");
    	run("Options...", "iterations=1 count=1 black do=Nothing");
        run("Set Measurements...", "area mean standard modal min bounding feret's area_fraction centroid rectangle circularity limit display redirect=Green_Droplets_Redirect decimal=3");
        run("Analyze Particles...", "size=10-Infinity circularity=0.85-1.00 show=Nothing display exclude include add exclude_holes exclude");
        

        // Optionally, save analyzed image
        saveAs("Tiff", analyzedDir + replace(fileName, ".tif", "_analyzed.tif"));
        // Save analyzed results to .csv in results folder
        saveAs("Results", outputDir + replace(fileName, ".tif", "_Droplet_stats.csv"));

        // Close all open windows
        close("*");
        close("ROI Manager");
        close("Results");
    }
}

