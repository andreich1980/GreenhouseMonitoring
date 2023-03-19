import { useEffect, useMemo, useRef, useState } from 'react'
import {
  Chart,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Legend,
  Title,
  Tooltip,
  TimeScale,
} from 'chart.js'
import { Line } from 'react-chartjs-2'
import { loadFileData, loadFilesList } from '../api'
import { CHART_DATA_TEMPLATE, getChartOptions } from '../helpers'

const Charts = () => {
  const [isLoading, setIsLoading] = useState(true)
  const [files, setFiles] = useState([])
  const [currentFileIndex, setCurrentFileIndex] = useState(null)
  const [chartData, setChartData] = useState(CHART_DATA_TEMPLATE)
  const [chartOptions, setChartOptions] = useState({})
  const [chartWidth, setChartWidth] = useState('100%')

  const chartRef = useRef(null)

  useEffect(() => {
    async function loadFileListAndPrepare() {
      try {
        const files = await loadFilesList('http://greenhouse.local/list')
        setFiles(files)
        setCurrentFileIndex(0)
      } catch (error) {
        console.error(error)
      } finally {
        setIsLoading(false)
      }
    }
    loadFileListAndPrepare()
  }, [])

  useEffect(() => {
    async function loadFileDataAndPrepare() {
      if (currentFileIndex == null || !files[currentFileIndex]) {
        return
      }

      setIsLoading(true)
      try {
        const jsonData = await loadFileData(
          `http://greenhouse.local/data/${files[currentFileIndex]}`,
        )

        const chartData = jsonData.reduce(
          (data, { time, temperature, humidity }) => {
            data.labels.push(time)
            data.datasets[0].data.push({ x: time, y: temperature })
            data.datasets[1].data.push({ x: time, y: humidity })
            return data
          },
          CHART_DATA_TEMPLATE,
        )
        chartData.xAxisTitle = new Date(
          files[currentFileIndex].split('.')[0],
        ).toLocaleDateString()

        setChartData(chartData)
        setChartOptions(getChartOptions(chartData.xAxisTitle, chartData.labels))
        setChartWidth(
          chartData.labels.length > 15
            ? chartData.labels.length * 30 + 'px'
            : '100%',
        )

        chartRef.current.update()
      } catch (error) {
        console.error(error)
      } finally {
        setIsLoading(false)
      }
    }
    loadFileDataAndPrepare()
  }, [currentFileIndex])

  if (isLoading) {
    return <div className="text-center italic text-gray-500">Loading...</div>
  }

  if (!files.length) {
    return <div>Theres no files with data</div>
  }

  Chart.register(
    CategoryScale,
    LinearScale,
    TimeScale,
    Legend,
    Title,
    Tooltip,
    PointElement,
    LineElement,
  )

  return (
    <div className="w-full overflow-x-auto">
      <div className="mx-auto h-96" style={{ width: chartWidth }}>
        <Line ref={chartRef} data={chartData} options={chartOptions} />
      </div>
    </div>
  )
}

export default Charts
