import {useEffect, useRef, useState} from 'react'
import p5 from 'p5'

export default function P5Canvas() {
  const ref = useRef()
  const [sketchKey, setSketchKey] = useState(0)

  useEffect(() => {
    while (ref.current.firstChild) {
      ref.current.removeChild(ref.current.firstChild)
    }

    const sketch = (p) => {
      let points = []

      p.setup = async () => {
        p.createCanvas(480, 480)
        p.background(240)

        // Load and parse SVG
        const raw = await fetch('/sample.svg').then(res => res.text())
        const doc = new DOMParser().parseFromString(raw, 'image/svg+xml')
        const path = doc.querySelector('path')

        // Append to DOM temporarily if needed (not required in most cases)
        document.body.appendChild(path) // optional if needed for getPointAtLength()

        const totalLength = path.getTotalLength()
        const step = 2

        for (let i = 0; i < totalLength; i += step) {
          const pt = path.getPointAtLength(i)
          points.push({x: pt.x, y: pt.y})
        }

        if (path.parentNode) path.remove() // cleanup if added

        // Draw as polyline
        p.noFill()
        p.stroke(0)
        p.beginShape()
        for (const pt of points) {
          p.vertex(pt.x, pt.y)
        }
        p.endShape()

        // Draw points
        p.fill(255, 0, 0)
        p.noStroke()
        for (const pt of points) {
          p.circle(pt.x, pt.y, 2)
        }
      }
    }

    const instance = new p5(sketch, ref.current)
    return () => instance.remove()
  }, [sketchKey])

  return (
    <>
      <button onClick={() => setSketchKey(k => k + 1)}>Reload Sketch</button>
      <div ref={ref}/>
    </>
  )
}
