import { leadingZero, sortDesc } from './helpers'

/**
 * Load files list from the given URL, sort them in descending order
 * @param {string} url
 * @returns {Array}
 */
export const loadFilesList = async (url) => {
  const files = await (await fetch(url)).json()
  return sortDesc(files)
}

/**
 * Load JSON Lines data from the given URL, convert them to JSON array,
 * fill `date` and `time` fields based on `timestamp` field.
 * @param {string} url
 * @returns {Object[]}
 */
export const loadFileData = async (url) => {
  const data = await (await fetch(url)).text()
  const jsonData = JSON.parse(
    `[${data
      .split('\n')
      .filter((row) => row)
      .join(',')}]`,
  )
  return jsonData.map((row) => {
    const date = new Date(row.timestamp)
    const hours = leadingZero(date.getHours())
    const minutes = leadingZero(date.getMinutes())
    const time = `${hours}:${minutes}`
    row.date = date.toLocaleDateString()
    row.time = time
    return row
  })
}
