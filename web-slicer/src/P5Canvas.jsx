import {useEffect, useRef, useState} from 'react'
import p5 from 'p5'

export default function P5Canvas() {
  const ref = useRef()
  const [sketchKey, setSketchKey] = useState(0)
  const [gcode, setGcode] = useState('');

  useEffect(() => {
    while (ref.current.firstChild) {
      ref.current.removeChild(ref.current.firstChild)
    }

    const sketch = (p) => {
      let points = []

      const minDistance = 100
      const maxDistance = 360
      const armLen = 160
      const fullSteps = 2900
      const fullDegrees = 200

      const drawXYLines = () => {
        p.strokeWeight(1)
        p.stroke(0, 255, 0)
        p.line(-p.width, 0, p.width, 0)
        p.stroke(255, 0, 0)
        p.line(0, 0, 0, p.height)
      }

      const drawRange = () => {
        p.push()
        p.noStroke()
        p.angleMode(p.DEGREES)

        p.fill(210, 255, 210)
        p.arc(
          0, 0,                      // center
          maxDistance * 2, maxDistance * 2,  // diameter
          -100 + 90,                 // start angle (shift so 0° is up)
          +100 + 90,                 // end angle
          p.PIE                    // close to center
        )

        // Inner cutout: white
        p.fill(240)
        p.arc(
          0, 0,
          minDistance * 2, minDistance * 2,
          -100 + 90,
          +100 + 90,
          p.PIE
        )

        p.angleMode(p.RADIANS)
        p.pop()
      }

      const screenToWorld = (x, y) => ({
        x: x - p.width / 2,
        y: -(y - (p.height - 100))
      })

      /**
       * For a 2-segment rhombus arm of length armLen, centered at (0,0):
       * @param {number} x        target X in world coords
       * @param {number} y        target Y in world coords
       * @param {number} armLen   length of each arm segment
       * @param {number} minDist  minimum radius
       * @param {number} maxDist  maximum radius
       * @param {number} fullSteps total steps per fullDegrees sweep
       * @param {number} fullDeg  mechanical sweep in degrees
       * @returns {object} { inRange, validArmsPositions, joints: [{x,y},{x,y}], angles: {alphaRad,alphaDeg,betaRad,betaDeg}, steps:{aSteps,bSteps} }
       */
      const computeRhombusKinematics = (x, y, armLen, minDist, maxDist, fullSteps, fullDeg) => {
        const d = Math.hypot(x, y)
        const inRange = (d >= minDist) && (d <= maxDist) && (d <= 2 * armLen)

        // compute joints
        const halfD = d / 2
        const h = (d <= 2 * armLen) ? Math.sqrt(armLen * armLen - halfD * halfD) : 0
        const ux = (d > 0) ? -y / d : 0
        const uy = (d > 0) ? x / d : 0
        const j1x = x / 2 + ux * h, j1y = y / 2 + uy * h
        const j2x = x / 2 - ux * h, j2y = y / 2 - uy * h

        // angles from vertical
        const alphaRad = Math.atan2(j1x, j1y)
        const betaRad = Math.atan2(j2x, j2y)
        const alphaDeg = alphaRad * 180 / Math.PI
        const betaDeg = betaRad * 180 / Math.PI

        const validArmsPositions = !(Math.abs(alphaDeg) > 100 || Math.abs(betaDeg) > 100);

        // steps conversion (inverted)
        const stepsPerDeg = fullSteps / fullDeg
        const aSteps = Math.round(-alphaDeg * stepsPerDeg)
        const bSteps = Math.round(-betaDeg * stepsPerDeg)

        return {
          inRange,
          validArmsPositions,
          joints: [
            {x: j1x, y: j1y},
            {x: j2x, y: j2y}
          ],
          angles: {alphaRad, alphaDeg, betaRad, betaDeg},
          steps: {aSteps, bSteps}
        }
      }

      function sliceAndPrintPath() {
        const entries = [];
        for (const pt of points) {
          if (!pt) {
            entries.push('{ 32767, 32767 }');
            continue;
          }

          const kin = computeRhombusKinematics(
            pt.x, pt.y,
            armLen, minDistance, maxDistance,
            fullSteps, fullDegrees
          );

          const a = kin.inRange && kin.validArmsPositions ? kin.steps.aSteps : 0;
          const b = kin.inRange && kin.validArmsPositions ? kin.steps.bSteps : 0;
          entries.push(`{ ${a}, ${b} }`);
        }

        let builder = "";

        builder += "#ifndef GCODE_H\n";
        builder += "#define GCODE_H\n";
        builder += "#include <Arduino.h>\n\n";
        builder += "// Auto-generated path steps (A, B):\n";
        builder += "const int pathLength = " + entries.length + ";\n";
        builder += "const int16_t pathSteps[pathLength][2] = {\n";
        entries.forEach(line => builder += "  " + line + ",\n");
        builder += "};\n";
        builder += "#endif //GCODE_H\n";

        setGcode(builder)
      }

      p.setup = async () => {
        p.createCanvas(640, 480)
        p.background(240)

        const offsetX = -95;
        const offsetY = 320;

        // Load & parse SVG path
        const raw = await fetch('/PP.svg').then(res => res.text())
        const doc = new DOMParser().parseFromString(raw, 'image/svg+xml')
        const paths = doc.querySelectorAll('path');
        for (const path of paths) {
          const totalLength = path.getTotalLength();
          const step = 2;
          for (let i = 0; i < totalLength; i += step) {
            const pt = path.getPointAtLength(i);
            points.push({ x: pt.x + offsetX, y: -pt.y + offsetY});
          }
          points.push(null); // pen up
        }

        sliceAndPrintPath();
      }

      p.draw = () => {
        p.background(240)

        // set origin & invert Y
        p.push()
        p.translate(p.width / 2, p.height - 100)
        p.scale(1, -1)

        drawRange()
        drawXYLines()

        // SVG polyline
        p.noFill()
        p.stroke(0)
        p.beginShape();
        for (const pt of points) {
          if (!pt) {
            p.endShape();
            p.beginShape();
            continue;
          }
          p.vertex(pt.x, pt.y);
        }
        p.endShape();

        // SVG sample points
        p.fill(255, 0, 0)
        p.noStroke()
        for (const pt of points) {
          if (!pt) continue;
          p.circle(pt.x, pt.y, 2);
        }

        // drawMouse();

// 3) get mouse in world coords
        const {x, y} = screenToWorld(p.mouseX, p.mouseY)

// 4) compute kinematics
        const kin = computeRhombusKinematics(
          x, y,
          armLen,
          minDistance,
          maxDistance,
          fullSteps,
          fullDegrees
        )

// 5) draw target dot
        p.fill(kin.inRange && kin.validArmsPositions ? 'black' : 'red')
        p.noStroke()
        p.circle(x, y, 8)

        if (kin.inRange) {
          // pull out joints, angles & steps
          const [j1, j2] = kin.joints
          const {alphaDeg, betaDeg} = kin.angles
          const {aSteps, bSteps} = kin.steps

          // draw arms
          p.strokeWeight(3)
          p.stroke(0, 100, 200)         // blue A
          p.line(0, 0, j1.x, j1.y)
          p.line(j1.x, j1.y, x, y)

          p.stroke(200, 50, 50)         // red B
          p.line(0, 0, j2.x, j2.y)
          p.line(j2.x, j2.y, x, y)

          // middle line
          p.stroke(0, 200, 0)
          p.line(0, 0, x, y)

          // print α/β + steps
          p.pop()
          p.push()
          p.resetMatrix()
          p.fill(0)
          p.textSize(12)
          p.text(`α: ${alphaDeg.toFixed(1)}° | ${aSteps} steps`, 10, 20)
          p.text(`β: ${betaDeg.toFixed(1)}° | ${bSteps} steps`, 10, 36)
          p.pop()
        }
      }
    }

    const instance = new p5(sketch, ref.current)
    return () => instance.remove()
  }, [sketchKey])

  return (
    <>
      <button onClick={() => setSketchKey(k => k + 1)}>Reload Sketch</button>
      <button onClick={() => {
        navigator.clipboard.writeText(gcode).then(() => {
          console.log('G-code copied to clipboard!');
        });
      }}>Copy G-code to Clipboard</button>
      <div ref={ref}/>
    </>
  )
}
