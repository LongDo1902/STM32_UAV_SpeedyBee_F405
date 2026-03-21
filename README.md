There are two required formatting files: **long_style_force_3_blank_lines_btw_func.py** and **long_style.xml**. Both are compulsory, so please follow the steps below.

1. Open STM32CubeIDE.
2. Go to Window -> Preferences -> C/C++ -> Code Style -> Formatter.
3. In the Formatter window, click Import.
4. Select long_style.xml, which is located in the project root directory.
5. Click Apply and Close.
6. Open your source code file, press Ctrl + A to select all, then press Ctrl + Shift + F to format the file.

Note:
At this stage, the blank line spacing between functions may still be only 1 line, which does not match our preferred style. To force 3 blank lines between functions, continue with the steps below.

7. Open Windows PowerShell at the project root directory.
8. Make sure you are inside STM32_UAV_SpeedyBee_F405 before running the command.
	Run the following command:
	Get-ChildItem Core\Src, Core\Inc -Recurse -File | Where-Object {
	    $_.Extension -in '.c', '.h'
	} | ForEach-Object {
	    py .\long_style_force_3_blank_lines_btw_func.py -i $_.FullName
	}