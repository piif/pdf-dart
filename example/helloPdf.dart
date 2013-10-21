
import 'package:pdfdart/pdflib.dart';

void main() {
    PDF pdf = new PDF("hello.pdf");

	pdf.setInfo("Creator", "helloPdf.dart");
	pdf.setInfo("Author", "Christian Lefebvre");
	pdf.setInfo("Title", "Hello, Dart world !");

	pdf.beginPage(PDF.pageSize["a4"]); // ~ 600x800

	var font = pdf.loadFont("Helvetica-Bold");

	pdf.setFont(font, 24);
	pdf.textTo("Hello", 50, 600, "fillcolor={rgb 0.5 0.2 0.5}");
	pdf.textTo(", world !!");
	pdf.setFont(font, 20);
	pdf.textTo("\nsays Dart");
	pdf.setColor("fill", 0, 0, 1);
	pdf.textTo(" on ${new DateTime.now()}");

	pdf.setLineWidth(2.0);
	pdf.setColor("stroke", 1.0, 0.2, 0.2);
	pdf.setColor("fill", 0.0, 0.85, 0.85);
	pdf.textTo("\nsome stars ...");

	etoile(pdf, 200, 400, PDF.FILL | PDF.CLOSE);
	etoile(pdf, 400, 400, PDF.STROKE | PDF.CLOSE);
	etoile(pdf, 200, 200, PDF.FILL_STROKE);
	etoile(pdf, 400, 200, PDF.FILL);

	var image1 = pdf.loadImage("chabenet.jpg");
	var image2 = pdf.loadImage("chabenet.png");

	pdf.imageTo(image1, 50, 650,
		"boxsize={200 150} position={center} fitmethod=meet showborder");
	pdf.imageTo(image2, 350, 650,
		"scale=0.22 boxsize={150 100} position={right top} fitmethod=clip showborder");

	pdf.endPage();

	pdf.close();
}

void etoile(PDF pdf, num x, num y, what) {
pdf..save()
		..translate(x, y)
		..moveTo(0, 100)
		..lineTo(40, -80)
		..curveTo(60, -60, -55, 50, -75, 50)
		..curveTo(-100, 30, 100, 30, 75, 50)
		..curveTo(55, 50, -60, -60, -40, -80)
		..terminatePath(what)
		..restore();
}
