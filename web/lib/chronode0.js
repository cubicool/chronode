// Do your Node requires here.
if(typeof exports !== "undefined") {
	// global.Papa = require("./papaparse.min.js");
}

((exports) => {
	exports.version = [0, 0, 0];

	exports.test = (data) => {},

	exports.chronode = function(c, width, height, profile) {
		// A Profile has: unit, depth, duration, data. It contains a variable number of Timer
		// objects, each of which has the same layout.
		const timer = profile.data[0];
		const start = timer.start;
		const stop = timer.stop;
		const PPU = width / (stop - start);
		const rows = profile.depth + 1;
		const rowHeight = height / rows;

		console.log(`Recursing: ${timer.name}`);
		console.log(`Range: start=${start}, stop=${stop}`);
		console.log(`Span: ${stop - start}ms`);
		// console.log(`PPU (Pixels Per Unit): ${canvas.width / (stop - start)}px`);
		console.log(`PPU (Pixels Per Unit): ${width / (stop - start)}px`);
		console.log(`Rows: ${rows}`);

		for(let i = 0; i < rows; i++) {
			console.log(`Drawing row ${i}...`);

			const yStart = i * rowHeight;
			const yEnd = yStart + rowHeight;

			// c.fillStyle = `rgba(100, 150, 255, 1)`;
			c.fillStyle = `rgba(200, 200, 200, 1)`;
			c.fillRect(0, yStart, width, yEnd - yStart);

			const centerX = width / 2;
			const centerY = yStart + rowHeight / 2;

			/* c.beginPath();
			c.arc(centerX, centerY, 10, 0, 2 * Math.PI);
			c.fillStyle = "red";
			c.fill(); */

			c.beginPath();
			c.moveTo(0, yEnd);
			c.lineTo(width, yEnd);
			c.strokeStyle = "white";
			c.lineWidth = 2;
			c.stroke();
		}

		function _chronode0(n, depth=0) {
			const y = Math.round((depth * rowHeight) + (rowHeight / 2));
			const x0 = Math.round((n.start - start) * PPU);
			const x1 = Math.round((n.stop - start) * PPU);

			console.log(`_chronde: ${n.name} @ ${depth} (x0=${x0}, x1=${x1}, y=${y})`);

			c.beginPath();
			c.moveTo(x0, y);
			c.lineTo(x1, y);
			c.strokeStyle = "red";
			c.lineWidth = 2;
			c.setLineDash([3, 1]);
			c.stroke();

			for(var x of [x0, x1]) {
				// console.log(`Doing: x=${x}, y=${y}`);

				c.beginPath();
				c.arc(x, y, 3, 0, 2 * Math.PI);
				c.fillStyle = "red";
				c.fill();
				c.closePath();
			}

			// https://stackoverflow.com/questions/15661339/how-do-i-fix-blurry-text-in-my-html5-canvas
			c.textAlign = "center";
			c.font = "10pt Sans";

			c.save(); {
				c.translate(0.5, 0.5);

				c.fillStyle = "#666";

				c.fillText(n.name, x0 + Math.round(((x1 - x0) / 2)), y - 5);
				c.fillText(`${n.stop - n.start}ms`, x0 + Math.round(((x1 - x0) / 2)), y + 14);

				// c.restore();

				c.fillText(n.name, x0 + Math.round(((x1 - x0) / 2)), y - 5);
				c.fillText(`${n.stop - n.start}ms`, x0 + Math.round(((x1 - x0) / 2)), y + 14);
			} c.restore();

			for(let i = 0; i < n.children.length; i++) _chronode0(n.children[i], depth + 1);
		}

		_chronode0(timer, 0);

		// canvasScalePPI(canvas.width, canvas.height);
	}
})(typeof exports === "undefined" ? this["chronode0"] = {} : exports);
